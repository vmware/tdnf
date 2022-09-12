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
    DEFAULT_KEY = 'file:///etc/pki/rpm-gpg/RPM-GPG-KEY-fedora-34-primary'
else:
    DEFAULT_KEY = 'file:///etc/pki/rpm-gpg/VMWARE-RPM-GPG-KEY'


@pytest.fixture(scope='function', autouse=True)
def setup_test_function(utils):
    utils.run(['rpm', '-e', '--allmatches', 'gpg-pubkey'])
    pkgname = utils.config["sglversion_pkgname"]
    utils.run(['tdnf', 'erase', '-y', pkgname])
    yield
    teardown_test(utils)


def teardown_test(utils):
    set_gpgcheck(utils, False)
    utils.run(['rpm', '-e', '--allmatches', 'gpg-pubkey'])
    pkgname = utils.config["sglversion_pkgname"]
    utils.run(['tdnf', 'erase', '-y', pkgname])


def set_gpgcheck(utils, enabled):
    tdnf_repo = os.path.join(utils.tdnf_config.get('main', 'repodir'), 'photon-test.repo')
    if enabled:
        utils.run(['sed', '-i', '/gpgcheck/s/.*/gpgcheck=1/g', tdnf_repo])
    else:
        utils.run(['sed', '-i', '/gpgcheck/s/.*/gpgcheck=0/g', tdnf_repo])


def set_repo_key(utils, url):
    tdnf_repo = os.path.join(utils.tdnf_config.get('main', 'repodir'), 'photon-test.repo')
    utils.run(['sed', '-i', '/gpgkey/s/.*/#gpgkey=/g', tdnf_repo])
    if url:
        utils.run(['sed', '-i', '/gpgkey/s@.*@gpgkey={}@g'.format(url), tdnf_repo])


# 'wrong' key in repo config, but skip signature, expect success
def test_install_skipsignature(utils):
    set_gpgcheck(utils, True)
    set_repo_key(utils, DEFAULT_KEY)
    pkgname = utils.config["sglversion_pkgname"]
    ret = utils.run(['tdnf', 'install', '-y', '--skipsignature', pkgname])
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
    set_repo_key(utils, DEFAULT_KEY)
    pkgname = utils.config["sglversion_pkgname"]
    ret = utils.run(['tdnf', 'install', '-y', pkgname])
    assert ret['retval'] == 1514
    assert not utils.check_package(pkgname)
