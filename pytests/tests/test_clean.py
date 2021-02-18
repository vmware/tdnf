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
    assert (ret['retval'] == 903)

def test_clean_invalid_arg(utils):
    ret = utils.run([ 'tdnf', 'clean', 'abcde' ])
    assert (ret['retval'] == 901)

def test_clean_packages(utils):
    ret = utils.run([ 'tdnf', 'clean', 'packages' ])
    assert (ret['retval'] == 1016)

def test_clean_dbcache(utils):
    ret = utils.run([ 'tdnf', 'clean', 'dbcache' ])
    assert (ret['retval'] == 1016)

def test_clean_metadata(utils):
    ret = utils.run([ 'tdnf', 'clean', 'metadata' ])
    assert (ret['retval'] == 1016)

def test_clean_expire_cache(utils):
    ret = utils.run([ 'tdnf', 'clean', 'expire-cache' ])
    assert (ret['retval'] == 1016)

def test_clean_plugins(utils):
    ret = utils.run([ 'tdnf', 'clean', 'plugins' ])
    assert (ret['retval'] == 1016)

def test_clean_all(utils):
    utils.run(['tdnf', 'makecache'])
    ret = utils.run([ 'tdnf', 'clean', 'all' ])
    assert (ret['retval'] == 0)

def test_clean_all_clean_already(utils):
    utils.run(['tdnf', 'makecache'])
    utils.run([ 'tdnf', 'clean', 'all' ])
    ret = utils.run([ 'tdnf', 'clean', 'all' ])
    assert (ret['retval'] == 0)

def test_clean_install_and_clean(utils):
    utils.run(['tdnf', 'makecache'])
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)
    utils.run([ 'tdnf', 'install', '-y', '--nogpgcheck', pkgname ])

    ret = utils.run([ 'tdnf', 'clean', 'all' ])
    assert (ret['retval'] == 0)

