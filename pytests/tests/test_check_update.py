#
# Copyright (C) 2019 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#
#   Author: Siddharth Chandrasekaran <csiddharth@vmware.com>

import pytest

@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)

def teardown_test(utils):
    pass

def test_check_update_no_arg(utils):
    ret = utils.run([ 'tdnf', 'check-update' ])
    assert (ret['retval'] == 0)

def test_check_update_invalid_args(utils):
    ret = utils.run([ 'tdnf', 'check-update', 'abcd', '1234' ])
    assert (ret['retval'] == 0)

def test_check_update_multi_version_package(utils):
    package = utils.config["mulversion_pkgname"] + '-' + utils.config["mulversion_lower"]
    ret = utils.run([ 'tdnf', 'install', '-y', '--nogpgcheck', package ])
    assert (utils.check_package(utils.config["mulversion_pkgname"]) == True)

    ret = utils.run([ 'tdnf', 'check-update', utils.config["mulversion_pkgname"] ])
    assert (len(ret['stdout']) > 0)

def test_check_update_single_version_package(utils):
    package = utils.config["sglversion_pkgname"]
    ret = utils.run([ 'tdnf', 'install', '-y', '--nogpgcheck', package ])
    assert (ret['retval'] == 0)

    ret = utils.run([ 'tdnf', 'check-update', package ])
    assert (len(ret['stdout']) == 0)
