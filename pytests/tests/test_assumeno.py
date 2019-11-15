#            (c) 2019 VMware Inc.,
#
#   Author: Siddharth Chandrasekaran <csiddharth@vmware.com>

import pytest

@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    # remove sglversion_pkgname at the beginning of tests
    pkgname = utils.config['sglversion_pkgname']
    utils.run([ 'tdnf', 'erase', '-y', pkgname ])
    yield
    teardown_test(utils)

def teardown_test(utils):
    pass

def test_assumeno_install(utils):
    pkgname = utils.config['sglversion_pkgname']
    utils.run([ 'tdnf', '--assumeno', 'install', pkgname ])
    assert (utils.check_package(pkgname) == False)

def test_assumeno_erase(utils):
    pkgname = utils.config['sglversion_pkgname']
    utils.run([ 'tdnf', '--assumeno',  'erase', pkgname ])
    assert (utils.check_package(pkgname) == False)
