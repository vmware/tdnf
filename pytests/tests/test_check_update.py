#            (c) 2019 VMware Inc.,
#
#   Author: Siddharth Chandrasekaran <csiddharth@vmware.com>

import pytest

@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    utils.run([ 'sed', '-i', 's/enabled=0/enabled=1/g', '/etc/yum.repos.d/photon-extras.repo' ])
    yield
    teardown_test(utils)

def teardown_test(utils):
    utils.run([ 'sed', '-i', 's/enabled=1/enabled=0/g', '/etc/yum.repos.d/photon-extras.repo' ])
    pass

def test_check_update_no_arg(utils):
    ret = utils.run([ 'tdnf', 'check-update' ])
    assert (ret['retval'] == 0)

def test_check_update_invalid_args(utils):
    ret = utils.run([ 'tdnf', 'check-update', 'abcd', '1234' ])
    assert (ret['retval'] == 0)

@pytest.mark.skip(reason='FIXME: chooses a suitbale package that works in Docker')
def test_check_update_multi_version_package(utils):
    package = utils.config["mulversion_pkgname"] + '-' + utils.config["mulversion_lower"]
    ret = utils.run([ 'tdnf', 'install', '-y', '--nogpgcheck', package ])
    assert (ret['retval'] == 0)

    ret = utils.run([ 'tdnf', 'check-update', utils.config["mulversion_pkgname"] ])
    assert (len(ret['stdout']) > 0)

def test_check_update_single_version_package(utils):
    package = utils.config["sglversion_pkgname"]
    ret = utils.run([ 'tdnf', 'install', '-y', '--nogpgcheck', package ])
    assert (ret['retval'] == 0)

    ret = utils.run([ 'tdnf', 'check-update', package ])
    assert (len(ret['stdout']) == 0)
