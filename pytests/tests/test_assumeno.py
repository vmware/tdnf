#            (c) 2019 VMware Inc.,
#
#   Author: Siddharth Chandrasekaran <csiddharth@vmware.com>

import pytest

@pytest.fixture(scope='module', autouse=True)
def setup_test_cache(utils):
    yield
    teardown_test_cache(utils)

def teardown_test_cache(utils):
    pass

def test_assumeno_install(utils):
    pkgname = utils.config['sglversion_pkgname']
    utils.run([ 'tdnf', '--assumeno', 'install', pkgname ])
    assert (utils.check_package(pkgname) == True)

def test_assumeno_erase(utils):
    pkgname = utils.config['sglversion_pkgname']
    utils.run([ 'tdnf', '--assumeno',  'erase', pkgname ])
    assert (utils.check_package(pkgname) == True)
