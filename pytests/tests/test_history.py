#
# Copyright (C) 2021-2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import pytest
import os

HIST_DB_DIRS = ["/var/lib/tdnf", "/usr/lib/sysimage/tdnf"]


@pytest.fixture(scope='function', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    pkgs = ['tdnf-test-cleanreq-leaf1', 'tdnf-test-cleanreq-required']
    pkgs.append(utils.config["mulversion_pkgname"])
    pkgs.append(utils.config["sglversion_pkgname"])
    pkgs.append(utils.config["sglversion2_pkgname"])

    utils.run("tdnf remove -y " + " ".join(pkgs))


def run_hist_util_cmd(utils, cmd):
    hist_db_util = 'tdnf-history-util'
    if utils.config.get('build_dir'):
        hist_db_util = os.path.join(utils.config['build_dir'], 'bin', hist_db_util)
    else:
        # for tests during make check through rpm
        hist_db_util = f"/usr/libexec/tdnf/{hist_db_util}"

    return utils._run(f"{hist_db_util} {cmd}")


def test_tdnf_history_util_help(utils):
    ret = run_hist_util_cmd(utils, "")
    assert ret['retval']
    assert "Usage:" in "\n".join(ret['stdout'])
    assert "Commands:" in "\n".join(ret['stdout'])


def test_tdnf_history_util_init(utils):
    for path in HIST_DB_DIRS:
        file = f"{path}/history.db"
        if os.path.exists(file):
            os.remove(file)

    ret = run_hist_util_cmd(utils, "init")
    assert ret['retval'] == 0


def test_history_list(utils):
    pkgname = utils.config["mulversion_pkgname"]

    utils.erase_package(pkgname)

    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkgname])
    assert utils.check_package(pkgname)
    ret = utils.run(['tdnf', 'history'])
    assert ret['retval'] == 0
    # 'install' must be in history output
    assert 'install' in '\n'.join(ret['stdout'])

    ret = utils.run(['tdnf', 'history', '--info'])
    assert ret['retval'] == 0
    # pkgname must be in history info output
    assert pkgname in '\n'.join(ret['stdout'])

    utils.erase_package(pkgname)
    assert not utils.check_package(pkgname)
    ret = utils.run(['tdnf', 'history'])
    assert ret['retval'] == 0
    # 'erase' must be in history output
    assert 'erase' in '\n'.join(ret['stdout'])

    ret = utils.run(['tdnf', 'history'])
    assert ret['retval'] == 0
    last = ret['stdout'][-1].split()[0]

    ret = utils.run(['tdnf', 'history', '--reverse'])
    assert ret['retval'] == 0
    rev_last = ret['stdout'][-1].split()[0]
    rev_first = ret['stdout'][1].split()[0]

    assert (last == rev_first)
    assert (int(rev_last) < int(rev_first))


def test_history_rollback(utils):
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)

    ret = utils.run(['tdnf', 'history'])
    assert ret['retval'] == 0
    baseline = ret['stdout'][-1].split()[0]

    ret = utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkgname])
    assert ret['retval'] == 0
    assert utils.check_package(pkgname)

    ret = utils.run(['tdnf', 'history', '-y', 'rollback', '--to', baseline])
    assert ret['retval'] == 0


def test_history_undo(utils):
    pkgs = [utils.config["mulversion_pkgname"]]
    pkgs.append(utils.config["sglversion_pkgname"])
    pkgs.append(utils.config["sglversion2_pkgname"])

    ret = utils.run("tdnf remove -y " + " ".join(pkgs))
    assert ret['retval'] == 0

    ret = utils.run(['tdnf', 'history'])
    assert ret['retval'] == 0
    baseline = ret['stdout'][-1].split()[0]

    for pkg in pkgs:
        ret = utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkg])
        assert ret['retval'] == 0
        assert utils.check_package(pkg)

    # should undo install of pkgs[1]
    ret = utils.run(['tdnf', 'history', '-y', 'undo', '--from', str(int(baseline) + 2)])
    assert ret['retval'] == 0
    assert not utils.check_package(pkgs[1])


def test_history_undo_remove(utils):
    pkgname = utils.config["sglversion_pkgname"]

    utils.erase_package(pkgname)

    ret = utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkgname])
    assert ret['retval'] == 0
    assert utils.check_package(pkgname)

    ret = utils.run(['tdnf', 'remove', '-y', pkgname])
    assert ret['retval'] == 0
    assert not utils.check_package(pkgname)

    ret = utils.run(['tdnf', 'history'])
    assert ret['retval'] == 0
    baseline = ret['stdout'][-1].split()[0]

    # should undo remove of pkgname
    ret = utils.run(['tdnf', 'history', '-y', '--nogpgcheck', 'undo', '--from', str(int(baseline))])
    assert ret['retval'] == 0
    assert utils.check_package(pkgname)


def test_history_undo_downloadonly(utils):
    pkgname = utils.config["sglversion_pkgname"]

    utils.erase_package(pkgname)

    ret = utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkgname])
    assert ret['retval'] == 0
    assert utils.check_package(pkgname)

    ret = utils.run(['tdnf', 'remove', '-y', pkgname])
    assert ret['retval'] == 0
    assert not utils.check_package(pkgname)

    ret = utils.run(['tdnf', 'history'])
    baseline = ret['stdout'][-1].split()[0]

    # would undo remove of pkgname
    ret = utils.run(['tdnf', 'history', '-y', '--downloadonly', '--nogpgcheck', 'undo', '--from', str(int(baseline))])
    assert ret['retval'] == 0
    assert not utils.check_package(pkgname)

    # verify that there is no record of it either
    ret = utils.run(['tdnf', 'history'])
    assert baseline == ret['stdout'][-1].split()[0]


def test_history_undo_testonly(utils):
    pkgname = utils.config["sglversion_pkgname"]

    utils.erase_package(pkgname)

    ret = utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkgname])
    assert utils.check_package(pkgname)

    ret = utils.run(['tdnf', 'remove', '-y', pkgname])
    assert not utils.check_package(pkgname)

    ret = utils.run(['tdnf', 'history'])
    baseline = ret['stdout'][-1].split()[0]

    # would undo remove of pkgname
    ret = utils.run(['tdnf', 'history', '-y', '--testonly', '--nogpgcheck', 'undo', '--from', str(int(baseline))])
    assert ret['retval'] == 0
    assert not utils.check_package(pkgname)

    # verify that there is no record of it either
    ret = utils.run(['tdnf', 'history'])
    assert baseline == ret['stdout'][-1].split()[0]


def test_history_undo_multiple(utils):
    pkgs = [utils.config["mulversion_pkgname"]]
    pkgs.append(utils.config["sglversion_pkgname"])
    pkgs.append(utils.config["sglversion2_pkgname"])

    ret = utils.run("tdnf remove -y " + " ".join(pkgs))
    assert ret['retval'] == 0

    ret = utils.run(['tdnf', 'history'])
    baseline = ret['stdout'][-1].split()[0]

    for pkg in pkgs:
        ret = utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkg])
        assert ret['retval'] == 0
        assert utils.check_package(pkg)

    ret = utils.run(['tdnf', 'history', '-y', 'undo', '--from', str(int(baseline) + 1), '--to', str(int(baseline) + 3)])
    assert ret['retval'] == 0
    assert not utils.check_package(pkgs[0])
    assert not utils.check_package(pkgs[1])
    assert not utils.check_package(pkgs[2])


def test_history_redo(utils):
    pkgs = [utils.config["mulversion_pkgname"]]
    pkgs.append(utils.config["sglversion_pkgname"])
    pkgs.append(utils.config["sglversion2_pkgname"])

    ret = utils.run("tdnf remove -y " + " ".join(pkgs))
    assert ret['retval'] == 0

    ret = utils.run(['tdnf', 'history'])
    assert ret['retval'] == 0
    baseline = ret['stdout'][-1].split()[0]

    for pkg in pkgs:
        utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkg])
        assert utils.check_package(pkg)

    utils.erase_package(pkgs[1])
    assert not utils.check_package(pkgs[1])

    # should redo install of pkgs[1]
    utils.run(['tdnf', 'history', '-y', 'redo', '--from', str(int(baseline) + 2)])
    assert ret['retval'] == 0
    assert utils.check_package(pkgs[1])


def test_history_mark(utils):
    pkgname = utils.config["mulversion_pkgname"]
    utils.install_package(pkgname)

    ret = utils.run(['tdnf', 'mark', 'remove', pkgname])
    assert ret['retval'] == 0

    ret = utils.run(['tdnf', 'history'])
    assert ret['retval'] == 0
    trans_id = ret['stdout'][-1].split()[0]

    ret = utils.run(['tdnf', 'history', '-y', 'undo', trans_id])
    assert ret['retval'] == 0

    ret = utils.run(['tdnf', 'repoquery', '--userinstalled', pkgname])
    assert pkgname in "\n".join(ret['stdout'])


# redo may pull additional deps if they were already installed
# when the transaction was made, but were removed later. They
# should be pulled in and marked autoinstalled
def test_history_redo_and_autoinstall(utils):
    pkgname = 'tdnf-test-cleanreq-leaf1'
    pkgname_req = 'tdnf-test-cleanreq-required'

    utils.install_package(pkgname_req)
    utils.install_package(pkgname)

    ret = utils.run(['tdnf', 'history'])
    trans_id = ret['stdout'][-1].split()[0]

    utils.erase_package(pkgname)
    utils.erase_package(pkgname_req)

    ret = utils.run(['tdnf', 'history', '-y', 'redo', trans_id])
    assert ret['retval'] == 0

    ret = utils.run(['tdnf', 'repoquery', '--userinstalled', pkgname_req])
    assert pkgname_req not in "\n".join(ret['stdout'])


def test_history_id(utils):
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)

    # record id before install
    ret = utils.run(['tdnf', 'history', 'id'])
    assert ret['retval'] == 0
    id_before = int(ret['stdout'][0].strip())

    utils.install_package(pkgname)

    # id must have increased by exactly 1 after a single install transaction
    ret = utils.run(['tdnf', 'history', 'id'])
    assert ret['retval'] == 0
    id_after = int(ret['stdout'][0].strip())
    assert id_after == id_before + 1


def test_history_id_json(utils):
    import json

    ret = utils.run(['tdnf', '-j', 'history', 'id'])
    assert ret['retval'] == 0
    data = json.loads(ret['stdout'][0])
    assert 'Id' in data
    assert isinstance(data['Id'], int)
    assert data['Id'] > 0


def test_history_memcheck(utils):
    ret = utils.run_memcheck(['tdnf', 'history'])
    assert ret['retval'] == 0

    ret = utils.run_memcheck(['tdnf', 'history', '--info'])
    assert ret['retval'] == 0


def get_host_gpg_keys(utils):
    host_gpg_keys = []
    ret = utils._run("rpm -qa 'gpg-pubkey*'")
    if ret['retval'] == 0:
        host_gpg_keys = ret['stdout']

    return host_gpg_keys


# ignore gpg-pubkey packages since we cannot revert a removal
def test_history_pubkey_removed(utils):
    host_gpg_keys = get_host_gpg_keys(utils)

    keypath = os.path.join(utils.config['repo_path'], 'photon-test', 'keys', 'pubkey.asc')

    ret = utils.run(['rpm', '--import', keypath])
    assert ret['retval'] == 0

    ret = utils.run(['tdnf', 'history', 'update'])
    assert ret['retval'] == 0

    new_gpg_key = get_host_gpg_keys(utils)
    new_gpg_key = list(set(new_gpg_key) - set(host_gpg_keys))

    ret = utils.run(['tdnf', 'history'])
    assert ret['retval'] == 0
    baseline = ret['stdout'][-1].split()[0]

    for key in new_gpg_key:
        ret = utils.run(['rpm', '-ev', key])
        assert ret['retval'] == 0

    ret = utils.run(['tdnf', 'history', 'update'])
    assert ret['retval'] == 0

    ret = utils.run(['tdnf', 'history', '-y', 'rollback', '--to', baseline])
    assert ret['retval'] == 0

    for key in new_gpg_key:
        ret = utils.run(['rpm', '-q', key])
        assert ret['retval']


def test_history_pubkey_added(utils):
    host_gpg_keys = get_host_gpg_keys(utils)

    ret = utils.run(['tdnf', 'history'])
    assert ret['retval'] == 0
    baseline = ret['stdout'][-1].split()[0]

    keypath = os.path.join(utils.config['repo_path'], 'photon-test', 'keys', 'pubkey.asc')
    ret = utils.run(['rpm', '--import', keypath])
    assert ret['retval'] == 0

    ret = utils.run(['tdnf', 'history', 'update'])
    assert ret['retval'] == 0

    ret = utils.run(['tdnf', 'history', '-y', 'rollback', '--to', baseline])
    assert ret['retval'] == 0

    new_gpg_key = get_host_gpg_keys(utils)
    new_gpg_key = list(set(new_gpg_key) - set(host_gpg_keys))
    for key in new_gpg_key:
        ret = utils.run(['rpm', '-q', key])
        assert ret['retval'] == 0
        ret = utils._run(f"rpm -ev {key}")
        assert ret['retval'] == 0
