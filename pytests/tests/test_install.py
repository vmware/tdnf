#
# Copyright (C) 2019 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#
#   Author: Siddharth Chandrasekaran <csiddharth@vmware.com>

import os
import tempfile
import pytest

@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)

def teardown_test(utils):
    pass

def erase_package(utils, pkgname, pkgversion):
    utils.run([ 'tdnf', 'erase', '-y', pkgname ])
    assert(utils.check_package(pkgname) == False)

def test_install_no_arg(utils):
    ret = utils.run([ 'tdnf', 'install' ])
    assert(ret['retval'] == 1001)

def test_install_invalid_arg(utils):
    ret = utils.run([ 'tdnf', 'install', 'invalid_package' ])
    assert(ret['retval'] == 1011)

def test_install_package_with_version_suffix(utils):
    pkgname = utils.config["mulversion_pkgname"]
    pkgversion = utils.config["mulversion_lower"]
    utils.erase_package(pkgname)
    utils.run([ 'tdnf', 'install', '-y', '--nogpgcheck', pkgname + '-' + pkgversion ])
    assert(utils.check_package(pkgname) == True)

def test_install_package_without_version_suffix(utils):
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)

    utils.run([ 'tdnf', 'install', '-y', '--nogpgcheck', pkgname ])
    assert(utils.check_package(pkgname) == True)
