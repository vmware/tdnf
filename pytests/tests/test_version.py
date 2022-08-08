#
# Copyright (C) 2020-2022 VMware, Inc. All Rights Reserved.
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


def test_version(utils):
    expected_version_string = utils.config['project_name'] + ': ' + \
        utils.config['project_version']
    ret = utils.run(['tdnf', '--version'])
    assert ret['retval'] == 0
    assert ret['stdout'][0] == expected_version_string


# memcheck
def test_version_memcheck(utils):
    ret = utils.run_memcheck(['tdnf', '--version'])
    assert ret['retval'] == 0
