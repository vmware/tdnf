#
# Copyright (C) 2021-2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import pytest


@pytest.fixture(scope='function', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    pkgname1 = utils.config["mulversion_pkgname"]
    pkgname2 = utils.config["sglversion_pkgname"]
    pkgname3 = utils.config["sglversion2_pkgname"]
    for pkg in [pkgname1, pkgname2, pkgname3]:
        utils.erase_package(pkg)
    utils.erase_package('tdnf-test-cleanreq-leaf1')
    utils.erase_package('tdnf-test-cleanreq-required')


def test_history_list(utils):
    pkgname = utils.config["mulversion_pkgname"]

    utils.erase_package(pkgname)

    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkgname])
    assert(utils.check_package(pkgname))
    ret = utils.run(['tdnf', 'history'])
    assert(ret['retval'] == 0)
    # 'install' must be in history output
    assert('install' in '\n'.join(ret['stdout']))

    ret = utils.run(['tdnf', 'history', '--info'])
    assert(ret['retval'] == 0)
    # pkgname must be in history info output
    assert(pkgname in '\n'.join(ret['stdout']))

    utils.erase_package(pkgname)
    assert(not utils.check_package(pkgname))
    ret = utils.run(['tdnf', 'history'])
    assert(ret['retval'] == 0)
    # 'erase' must be in history output
    assert('erase' in '\n'.join(ret['stdout']))

    ret = utils.run(['tdnf', 'history'])
    last = ret['stdout'][-1].split()[0]

    ret = utils.run(['tdnf', 'history', '--reverse'])
    rev_last = ret['stdout'][-1].split()[0]
    rev_first = ret['stdout'][1].split()[0]

    assert (last == rev_first)
    assert (int(rev_last) < int(rev_first))


def test_history_rollback(utils):
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)

    ret = utils.run(['tdnf', 'history'])
    baseline = ret['stdout'][-1].split()[0]

    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkgname])
    assert(utils.check_package(pkgname))

    utils.run(['tdnf', 'history', '-y', 'rollback', '--to', baseline])
    assert(ret['retval'] == 0)


def test_history_undo(utils):
    pkgname1 = utils.config["mulversion_pkgname"]
    pkgname2 = utils.config["sglversion_pkgname"]
    pkgname3 = utils.config["sglversion2_pkgname"]

    for pkg in [pkgname1, pkgname2, pkgname2]:
        utils.erase_package(pkg)

    ret = utils.run(['tdnf', 'history'])
    baseline = ret['stdout'][-1].split()[0]

    for pkg in [pkgname1, pkgname2, pkgname3]:
        utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkg])
        assert(utils.check_package(pkg))

    # should undo install of pkgname2
    utils.run(['tdnf', 'history', '-y', 'undo', '--from', str(int(baseline) + 2)])
    assert(ret['retval'] == 0)
    assert(not utils.check_package(pkgname2))


def test_history_undo_multiple(utils):
    pkgname1 = utils.config["mulversion_pkgname"]
    pkgname2 = utils.config["sglversion_pkgname"]
    pkgname3 = utils.config["sglversion2_pkgname"]

    for pkg in [pkgname1, pkgname2, pkgname2]:
        utils.erase_package(pkg)

    ret = utils.run(['tdnf', 'history'])
    baseline = ret['stdout'][-1].split()[0]

    for pkg in [pkgname1, pkgname2, pkgname3]:
        utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkg])
        assert(utils.check_package(pkg))

    utils.run(['tdnf', 'history', '-y', 'undo', '--from', str(int(baseline) + 1), '--to', str(int(baseline) + 3)])
    assert(ret['retval'] == 0)
    assert(not utils.check_package(pkgname1))
    assert(not utils.check_package(pkgname2))
    assert(not utils.check_package(pkgname3))


def test_history_redo(utils):
    pkgname1 = utils.config["mulversion_pkgname"]
    pkgname2 = utils.config["sglversion_pkgname"]
    pkgname3 = utils.config["sglversion2_pkgname"]

    for pkg in [pkgname1, pkgname2, pkgname3]:
        utils.erase_package(pkg)

    ret = utils.run(['tdnf', 'history'])
    baseline = ret['stdout'][-1].split()[0]

    for pkg in [pkgname1, pkgname2, pkgname2]:
        utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkg])
        assert(utils.check_package(pkg))

    utils.erase_package(pkgname2)
    assert(not utils.check_package(pkgname2))

    # should redo install of pkgname2
    utils.run(['tdnf', 'history', '-y', 'redo', '--from', str(int(baseline) + 2)])
    assert(ret['retval'] == 0)
    assert(utils.check_package(pkgname2))


def test_history_mark(utils):
    pkgname = utils.config["mulversion_pkgname"]
    utils.install_package(pkgname)

    ret = utils.run(['tdnf', 'mark', 'remove', pkgname])
    assert(ret['retval'] == 0)

    ret = utils.run(['tdnf', 'history'])
    trans_id = ret['stdout'][-1].split()[0]

    utils.run(['tdnf', 'history', '-y', 'undo', trans_id])
    assert(ret['retval'] == 0)

    ret = utils.run(['tdnf', 'repoquery', '--userinstalled', pkgname])
    assert(pkgname in "\n".join(ret['stdout']))


# redo may pull additional deps if they were already installed
# when the transaction was made, but were removed later. They
# should to be pulled in and marked autoinstalled
def test_history_redo_and_autoinstall(utils):
    pkgname = 'tdnf-test-cleanreq-leaf1'
    pkgname_req = 'tdnf-test-cleanreq-required'

    utils.install_package(pkgname_req)
    utils.install_package(pkgname)

    ret = utils.run(['tdnf', 'history'])
    trans_id = ret['stdout'][-1].split()[0]

    utils.erase_package(pkgname)
    utils.erase_package(pkgname_req)

    utils.run(['tdnf', 'history', '-y', 'redo', trans_id])
    assert(ret['retval'] == 0)

    ret = utils.run(['tdnf', 'repoquery', '--userinstalled', pkgname_req])
    assert(pkgname_req not in "\n".join(ret['stdout']))


def test_history_memcheck(utils):
    ret = utils.run_memcheck(['tdnf', 'history'])
    assert(ret['retval'] == 0)

    ret = utils.run_memcheck(['tdnf', 'history', '--info'])
    assert(ret['retval'] == 0)
