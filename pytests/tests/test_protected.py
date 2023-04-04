#
# Copyright (C) 2021-2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import os
import pytest
import errno
import shutil

TESTREPO = 'photon-test'

PKGNAME_OBSED_VER = "tdnf-test-dummy-obsoleted=0.1"
PKGNAME_OBSED = "tdnf-test-dummy-obsoleted"
PKGNAME_OBSING = "tdnf-test-dummy-obsoleting"


@pytest.fixture(scope='function', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    dirname = os.path.join(utils.config['repo_path'], 'protected.d')
    if os.path.isdir(dirname):
        shutil.rmtree(dirname)

    utils.erase_package('tdnf-test-cleanreq-required')
    utils.erase_package('tdnf-test-cleanreq-leaf1')


# helper to create directory tree without complains when it exists:
def makedirs(d):
    try:
        os.makedirs(d)
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise


def set_protected_file(utils, value):
    dirname = os.path.join(utils.config['repo_path'], 'protected.d')
    makedirs(dirname)
    filename = os.path.join(dirname, 'test.conf')
    with open(filename, 'w') as f:
        f.write(value)


def test_protected_conf_erase(utils):
    pkgname = utils.config["sglversion_pkgname"]
    set_protected_file(utils, pkgname)
    utils.install_package(pkgname)

    # sanity check
    assert utils.check_package(pkgname)

    # test - uninstalling should fail
    utils.run(['tdnf', '-y', '--nogpgcheck', 'remove', pkgname])
    assert utils.check_package(pkgname)


def test_protected_required(utils):
    pkgname = 'tdnf-test-cleanreq-leaf1'
    pkgname_req = 'tdnf-test-cleanreq-required'

    utils.install_package(pkgname)
    assert utils.check_package(pkgname)
    assert utils.check_package(pkgname_req)

    set_protected_file(utils, pkgname)

    # would erase pkgname:
    ret = utils.run(['tdnf', '-y', '--nogpgcheck', 'remove', pkgname_req])
    assert ret['retval'] != 0
    assert utils.check_package(pkgname)
    assert utils.check_package(pkgname_req)


def test_protected_autoerase(utils):
    pkgname = 'tdnf-test-cleanreq-leaf1'
    pkgname_req = 'tdnf-test-cleanreq-required'

    utils.install_package(pkgname)
    assert utils.check_package(pkgname)
    assert utils.check_package(pkgname_req)

    set_protected_file(utils, pkgname_req)

    # would erase pkgname_req if not protected:
    ret = utils.run(['tdnf', '-y', '--nogpgcheck', 'autoremove', pkgname])
    assert ret['retval'] == 0
    assert utils.check_package(pkgname_req)


# install an obsoleting package, but the obsoleted package to be
# removed is protected
def test_protected_obsoleted(utils):
    utils.erase_package(PKGNAME_OBSING)
    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', PKGNAME_OBSED_VER])
    assert utils.check_package(PKGNAME_OBSED)

    set_protected_file(utils, PKGNAME_OBSED)

    # expected fail
    ret = utils.run(['tdnf', 'install', '-y', '--nogpgcheck', PKGNAME_OBSING])
    assert ret['retval'] == 1030
    assert utils.check_package(PKGNAME_OBSED)
