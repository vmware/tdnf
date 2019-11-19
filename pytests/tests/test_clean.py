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

def test_clean_no_args(utils):
    ret = utils.run([ 'tdnf', 'clean' ])
    assert (ret['retval'] == 135)

def test_clean_invalid_arg(utils):
    ret = utils.run([ 'tdnf', 'clean', 'abcde' ])
    assert (ret['retval'] == 133)

def test_clean_packages(utils):
    ret = utils.run([ 'tdnf', 'clean', 'packages' ])
    assert (ret['retval'] == 248)

def test_clean_dbcache(utils):
    ret = utils.run([ 'tdnf', 'clean', 'dbcache' ])
    assert (ret['retval'] == 248)

def test_clean_metadata(utils):
    ret = utils.run([ 'tdnf', 'clean', 'metadata' ])
    assert (ret['retval'] == 248)

def test_clean_expire_cache(utils):
    ret = utils.run([ 'tdnf', 'clean', 'expire-cache' ])
    assert (ret['retval'] == 248)

def test_clean_plugins(utils):
    ret = utils.run([ 'tdnf', 'clean', 'plugins' ])
    assert (ret['retval'] == 248)
