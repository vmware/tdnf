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


def test_provides_no_arg(utils):
    ret = utils.run(['tdnf', 'provides'])
    assert ret['retval'] == 907


def test_provides_valid_pkg_name(utils):
    ret = utils.run(['tdnf', 'provides', 'tdnf'])
    assert ret['retval'] == 0


def test_provides_invalid_pkg_name(utils):
    ret = utils.run(['tdnf', 'provides', 'invalid_pkg_name'])
    assert ret['stderr'][0] == 'No data available'
