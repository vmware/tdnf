#
# Copyright (C) 2019 - 2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.

import os
import pytest

DIST = os.environ.get('DIST')
if DIST == 'fedora':
    DEFAULT_KEY = 'file:///etc/pki/rpm-gpg/RPM-GPG-KEY-fedora-rawhide-primary'
else:
    DEFAULT_KEY = 'file:///etc/pki/rpm-gpg/VMWARE-RPM-GPG-KEY-4096'

original_gpg_keys = []


def get_host_gpg_keys(utils):
    host_gpg_keys = []
    ret = utils._run("rpm -qa 'gpg-pubkey*'")
    if ret['retval'] == 0:
        host_gpg_keys = ret['stdout']

    return host_gpg_keys


@pytest.fixture(scope='function', autouse=True)
def setup_test_function(utils):
    global original_gpg_keys
    if not original_gpg_keys:
        original_gpg_keys = get_host_gpg_keys(utils)

    new_gpg_key = get_host_gpg_keys(utils)
    new_gpg_key = [k for k in set(new_gpg_key) - set(original_gpg_keys) if not any(
        k.split('-')[2].endswith(orig.split('-')[2]) or orig.split('-')[2].endswith(k.split('-')[2])
        for orig in original_gpg_keys)]
    for key in new_gpg_key:
        ret = utils._run(f"rpm -ev {key}")
        assert ret['retval'] == 0

    pkgname = utils.config["sglversion_pkgname"]
    utils.run(['tdnf', 'erase', '-y', pkgname])
    yield
    teardown_test(utils)


def teardown_test(utils):
    set_gpgcheck(utils, False)
    set_gpgcheck(utils, False, None)

    new_gpg_key = get_host_gpg_keys(utils)
    new_gpg_key = [k for k in set(new_gpg_key) - set(original_gpg_keys) if not any(
        k.split('-')[2].endswith(orig.split('-')[2]) or orig.split('-')[2].endswith(k.split('-')[2])
        for orig in original_gpg_keys)]
    for key in new_gpg_key:
        ret = utils._run(f"rpm -ev {key}")
        assert ret['retval'] == 0

    pkgname = utils.config["sglversion_pkgname"]
    utils.run(['tdnf', 'erase', '-y', pkgname])


def set_gpgcheck(utils, enabled, repo='photon-test'):
    if enabled is not None:
        utils.edit_config({'gpgcheck': '1' if enabled else '0'}, repo)
    else:
        utils.edit_config({'gpgcheck': None}, repo)


def set_repo_key(utils, url):
    utils.edit_config({'gpgkey': url}, repo='photon-test')


# install unsigned package with gpgcheck enabled in repo,
# expect failure
def test_install_unsigned(utils):
    set_gpgcheck(utils, None, repo=None)
    set_gpgcheck(utils, True, repo='photon-test-unsigned')
    set_repo_key(utils, DEFAULT_KEY)
    pkgname = utils.config["sglversion_pkgname"]
    ret = utils.run(['tdnf', '--repoid', 'photon-test-unsigned', 'install', '-y', pkgname])
    assert ret['retval'] == 1531
    assert not utils.check_package(pkgname)


# install unsigned package with gpgcheck enabled in global config,
# expect failure
def test_install_unsigned_global_gpgcheck(utils):
    set_gpgcheck(utils, True, repo=None)
    set_gpgcheck(utils, None, repo='photon-test-unsigned')
    set_repo_key(utils, DEFAULT_KEY)
    pkgname = utils.config["sglversion_pkgname"]
    ret = utils.run(['tdnf', '--repoid', 'photon-test-unsigned', 'install', '-y', pkgname])
    assert ret['retval'] == 1531
    assert not utils.check_package(pkgname)


# install unsigned package with gpgcheck enabled in repo,
# but disabled on command line,
# expect success
def test_install_unsigned_nogpgcheck(utils):
    set_gpgcheck(utils, True, repo='photon-test-unsigned')
    set_repo_key(utils, DEFAULT_KEY)
    pkgname = utils.config["sglversion_pkgname"]
    ret = utils.run(['tdnf', '--nogpgcheck', '--repoid', 'photon-test-unsigned', 'install', '-y', pkgname])
    assert ret['retval'] == 0
    assert utils.check_package(pkgname)


# install unsigned package with gpgcheck enabled in repo,
# but skipsignature on command line,
# expect success
def test_install_unsigned_skipsignature(utils):
    set_gpgcheck(utils, True, repo='photon-test-unsigned')
    set_repo_key(utils, DEFAULT_KEY)
    pkgname = utils.config["sglversion_pkgname"]
    ret = utils.run(['tdnf', '--skipsignature', '--repoid', 'photon-test-unsigned', 'install', '-y', pkgname])
    assert ret['retval'] == 0
    assert utils.check_package(pkgname)


# 'wrong' key in repo config, but skip signature, expect success
def test_install_skipsignature(utils):
    set_gpgcheck(utils, True)
    set_repo_key(utils, DEFAULT_KEY)
    pkgname = utils.config["sglversion_pkgname"]
    ret = utils.run(['tdnf', 'install', '-y', '--skipsignature', pkgname])
    assert ret['retval'] == 0
    assert utils.check_package(pkgname)


def test_install_skipdigest(utils):
    set_gpgcheck(utils, True)
    keypath = os.path.join(utils.config['repo_path'], 'photon-test', 'keys', 'pubkey.asc')
    utils.run(['rpm', '--import', keypath])
    set_repo_key(utils, DEFAULT_KEY)
    pkgname = utils.config["sglversion_pkgname"]
    ret = utils.run(['tdnf', 'install', '-y', '--skipdigest', pkgname])
    assert ret['retval'] == 0
    assert utils.check_package(pkgname)


# import key prior to install, expect success
def test_install_with_key(utils):
    set_gpgcheck(utils, True)
    keypath = os.path.join(utils.config['repo_path'], 'photon-test', 'keys', 'pubkey.asc')
    set_repo_key(utils, DEFAULT_KEY)
    utils.run(['rpm', '--import', keypath])
    pkgname = utils.config["sglversion_pkgname"]
    ret = utils.run(['tdnf', 'install', '-y', pkgname])
    assert ret['retval'] == 0
    assert utils.check_package(pkgname)


# import local, correct key during install from repo config, expect success
def test_install_local_key(utils):
    set_gpgcheck(utils, True)
    keypath = os.path.join(utils.config['repo_path'], 'photon-test', 'keys', 'pubkey.asc')
    set_repo_key(utils, 'file://{}'.format(keypath))
    pkgname = utils.config["sglversion_pkgname"]
    ret = utils.run(['tdnf', 'install', '-y', pkgname])
    assert ret['retval'] == 0
    assert utils.check_package(pkgname)


# import remote, correct key during install from repo config, expect success
def test_install_remote_key(utils):
    set_gpgcheck(utils, True)
    set_repo_key(utils, 'http://localhost:8080/photon-test/keys/pubkey.asc')
    pkgname = utils.config["sglversion_pkgname"]
    ret = utils.run(['tdnf', 'install', '-y', pkgname])
    assert ret['retval'] == 0
    assert utils.check_package(pkgname)


# -v (verbose) prints progress data
def test_install_remote_key_verbose(utils):
    set_gpgcheck(utils, True)
    set_repo_key(utils, 'http://localhost:8080/photon-test/keys/pubkey.asc')
    pkgname = utils.config["sglversion_pkgname"]
    ret = utils.run(['tdnf', 'install', '-v', '-y', pkgname])
    assert ret['retval'] == 0
    assert utils.check_package(pkgname)


# import remote key with url containing a directory traversal, expect fail
def test_install_remote_key_no_traversal(utils):
    set_gpgcheck(utils, True)
    set_repo_key(utils, 'http://localhost:8080/../photon-test/keys/pubkey.asc')
    pkgname = utils.config["sglversion_pkgname"]
    ret = utils.run(['tdnf', 'install', '-y', pkgname])
    assert ret['retval'] != 0


# import remote key with url containing a directory traversal, expect fail
def test_install_remote_key_no_traversal2(utils):
    set_gpgcheck(utils, True)
    set_repo_key(utils, 'http://localhost:8080/photon-test/keys/../../../pubkey.asc')
    pkgname = utils.config["sglversion_pkgname"]
    ret = utils.run(['tdnf', 'install', '-y', pkgname])
    assert ret['retval'] != 0


# test with gpgcheck enabled but no key entry, expect fail
def test_install_nokey(utils):
    set_gpgcheck(utils, True)
    set_repo_key(utils, None)
    pkgname = utils.config["sglversion_pkgname"]
    ret = utils.run(['tdnf', 'install', '-y', pkgname])
    assert ret['retval'] == 1523
    assert not utils.check_package(pkgname)


# 'wrong' key in repo config, expect fail
def test_install_nokey1(utils):
    set_gpgcheck(utils, True)
    keypath = os.path.join(utils.config['repo_path'], 'photon-test', 'keys', 'pubkey.wrong.asc')
    set_repo_key(utils, f"file://{keypath}")
    pkgname = utils.config["sglversion_pkgname"]
    utils.run(['rpm', '--import', keypath])
    ret = utils.run(['tdnf', 'install', '-y', pkgname])
    assert ret['retval'] == 1514
    assert not utils.check_package(pkgname)
