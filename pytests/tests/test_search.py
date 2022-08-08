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


def test_search_no_arg(utils):
    ret = utils.run(['tdnf', 'search'])
    assert ret['retval'] == 1599


def test_search_invalid_arg(utils):
    ret = utils.run(['tdnf', 'search', 'invalid_arg'])
    assert ret['retval'] == 1599


def test_search_single(utils):
    ret = utils.run(['tdnf', 'search', 'tdnf'])
    assert ret['retval'] == 0


def test_search_multiple(utils):
    ret = utils.run(['tdnf', 'search', 'tdnf', 'wget', 'gzip'])
    assert ret['retval'] == 0
