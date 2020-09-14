#
# Copyright (C) 2020 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#
#   Author: Keerthana K <keerthanak@vmware.com>

import pytest

@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)

def teardown_test(utils):
    pass

def install_dummy_pretrans_dependency(utils):
    utils.run([ 'tdnf', 'install', '-y', 'tdnf-dummy-pretrans' ])

def remove_dummy_pretrans_dependency(utils):
    utils.run([ 'tdnf', 'remove', '-y', 'tdnf-dummy-pretrans' ])

def test_install_pretrans_lessthan_fail(utils):
    remove_dummy_pretrans_dependency(utils)
    ret = utils.run([ 'tdnf', 'install', '-y', 'tdnf-test-pretrans-one' ])
    assert(ret['stderr'][0].startswith('Detected rpm pre-transaction dependency'))
    assert(ret['retval'] == 1515)

def test_install_pretrans_greaterthan_fail(utils):
    remove_dummy_pretrans_dependency(utils)
    ret = utils.run([ 'tdnf', 'install', '-y', 'tdnf-test-pretrans-two' ])
    assert(ret['stderr'][0].startswith('Detected rpm pre-transaction dependency'))
    assert(ret['retval'] == 1515)

def test_install_pretrans_lessthan_success(utils):
    install_dummy_pretrans_dependency(utils)
    ret = utils.run([ 'tdnf', 'install', '-y', 'tdnf-test-pretrans-one' ])
    assert(ret['retval'] == 0)

def test_install_pretrans_greaterthan_success(utils):
    install_dummy_pretrans_dependency(utils)
    ret = utils.run([ 'tdnf', 'install', '-y', 'tdnf-test-pretrans-two' ])
    assert(ret['retval'] == 0)
