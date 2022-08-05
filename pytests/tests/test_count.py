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


def test_count(utils):
    ret = utils.run(['tdnf', 'count'])
    assert ret['retval'] == 0


# memcheck
def test_count_memcheck(utils):
    ret = utils.run_memcheck(['tdnf', 'count'])
    assert ret['retval'] == 0
