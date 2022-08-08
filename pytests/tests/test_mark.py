#
# Copyright (C) 2022 VMware, Inc. All Rights Reserved.
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


# mark autoinstalled package as user installed,
# should not be removed on autoremove
def test_mark_install(utils):
    pkgname = 'tdnf-test-cleanreq-leaf1'
    pkgname_req = 'tdnf-test-cleanreq-required'
    utils.install_package(pkgname)

    assert utils.check_package(pkgname_req)

    ret = utils.run(['tdnf', 'mark', 'install', pkgname_req])
    assert ret['retval'] == 0

    utils.run(['tdnf', '-y', 'autoremove', pkgname])

    assert not utils.check_package(pkgname)
    # actual test - pkg should still be there:
    assert utils.check_package(pkgname_req)


# dependency already installed, hence would not be removed
# on autoremove, but should be if we mark it auto installed:
def test_mark_remove(utils):
    pkgname = 'tdnf-test-cleanreq-leaf1'
    pkgname_req = 'tdnf-test-cleanreq-required'
    utils.install_package(pkgname_req)
    utils.install_package(pkgname)

    assert utils.check_package(pkgname_req)

    ret = utils.run(['tdnf', 'mark', 'remove', pkgname_req])
    assert ret['retval'] == 0

    utils.run(['tdnf', '-y', 'autoremove', pkgname])

    assert not utils.check_package(pkgname)
    # actual test - pkg should be removed:
    assert not utils.check_package(pkgname_req)
