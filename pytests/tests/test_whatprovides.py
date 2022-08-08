#
# Copyright (C) 2019-2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import pytest


@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    pass


def test_whatprovides_no_arg(utils):
    ret = utils.run(['tdnf', 'whatprovides'])
    assert ret['retval'] == 907


def test_whatprovides_invalid_arg(utils):
    ret = utils.run(['tdnf', 'whatprovides', 'invalid_arg'])
    assert ret['stderr'][0] == 'No data available'


def test_whatprovides_valid_file_notinstalled(utils):
    ret = utils.run(['tdnf', 'whatprovides', '/lib/systemd/system/tdnf-test-one.service'])
    assert 'tdnf-test-one' in "\n".join(ret['stdout'])
    assert ret['retval'] == 0


def test_whatprovides_valid_file_installed(utils):
    ret = utils.run(['tdnf', 'install', '-y', '--nogpgcheck', 'tdnf-test-one'])
    ret = utils.run(['tdnf', 'whatprovides', '/lib/systemd/system/tdnf-test-one.service'])
    assert 'tdnf-test-one' in "\n".join(ret['stdout'])
    assert ret['retval'] == 0
