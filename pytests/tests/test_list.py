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
    mpkg = utils.config["mulversion_pkgname"]
    spkg = utils.config["sglversion_pkgname"]
    utils.run(['tdnf', 'erase', '-y', mpkg, spkg])

def helper_test_list_sub_cmd(utils, sub_cmd):
    ret = utils.run([ 'tdnf', 'list', sub_cmd ])
    assert(ret['retval'] == 0)
    ret = utils.run([ 'tdnf', 'list', sub_cmd, utils.config["sglversion_pkgname"] ])
    assert(ret['retval'] == 0)
    ret = utils.run([ 'tdnf', 'list', sub_cmd, 'invalid_package' ])
    assert(ret['retval'] == 1011)

def test_list_top(utils):
    ret = utils.run([ 'tdnf', 'list' ])
    assert(ret['retval'] == 0)
    ret = utils.run([ 'tdnf', 'list', utils.config["sglversion_pkgname"] ])
    assert(ret['retval'] == 0)
    ret = utils.run([ 'tdnf', 'list', 'invalid_package' ])
    assert(ret['retval'] == 1011)

def test_list_sub_cmd(utils):
    spkg = utils.config["sglversion_pkgname"]
    ret = utils.run([ 'tdnf', 'install', '-y', spkg])
    assert(ret['retval'] == 0)

    helper_test_list_sub_cmd(utils, 'all')
    helper_test_list_sub_cmd(utils, 'installed')
    helper_test_list_sub_cmd(utils, 'available')
    helper_test_list_sub_cmd(utils, 'obsoletes')
    helper_test_list_sub_cmd(utils, 'extras')
    helper_test_list_sub_cmd(utils, 'recent')

def test_list_upgrades(utils):
    mpkg = utils.config["mulversion_pkgname"]
    mpkg_version = utils.config["mulversion_lower"]
    spkg = utils.config["sglversion_pkgname"]

    ret = utils.run([ 'tdnf', 'list', 'upgrades' ])

    # TODO Fix this; see issue #94
    if ret['retval'] == 1011 or ret['retval'] == 0:
        result = True
    else:
        result = False
    assert(result)

    utils.run([ 'tdnf', 'install', '-y', '--nogpgcheck', mpkg + '-' + mpkg_version ])
    ret = utils.run([ 'tdnf', 'list', 'upgrades', mpkg ])
    assert(len(ret['stdout']) == 1)

    ret = utils.run([ 'tdnf', 'list', 'upgrades', 'invalid_package' ])
    assert(ret['retval'] == 1011)

    ret = utils.run([ 'tdnf', 'list', 'upgrades', spkg ])
    assert(ret['retval'] == 1011)
