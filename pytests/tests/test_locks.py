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


@pytest.fixture(scope='function', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    dirname = os.path.join(utils.config['repo_path'], 'locks.d')
    if os.path.isdir(dirname):
        shutil.rmtree(dirname)

    utils.erase_package(utils.config["sglversion_pkgname"])
    utils.erase_package(utils.config["mulversion_pkgname"])


# helper to create directory tree without complains when it exists:
def makedirs(d):
    try:
        os.makedirs(d)
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise


def set_locks_file(utils, value):
    dirname = os.path.join(utils.config['repo_path'], 'locks.d')
    makedirs(dirname)
    filename = os.path.join(dirname, 'test.conf')
    with open(filename, 'w') as f:
        f.write(value)


def test_locks_conf_erase(utils):
    pkgname = utils.config["sglversion_pkgname"]
    set_locks_file(utils, pkgname)
    utils.install_package(pkgname)

    # sanity check
    assert utils.check_package(pkgname)

    # test - uninstalling should fail
    utils.run(['tdnf', '-y', '--nogpgcheck', 'remove', pkgname])
    assert utils.check_package(pkgname)


def test_locks_conf_update(utils):
    pkgname = utils.config["mulversion_pkgname"]
    version_low = utils.config["mulversion_lower"]
    set_locks_file(utils, pkgname)

    utils.install_package(pkgname, pkgversion=version_low)
    assert utils.check_package(pkgname)

    # test - update should fail
    utils.run(['tdnf', '-y', '--nogpgcheck', 'update', pkgname])
    assert utils.check_package(pkgname, version=version_low)
