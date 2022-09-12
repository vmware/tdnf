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


def test_repolist(utils):
    ret = utils.run(['tdnf', 'repolist'])
    assert ret['retval'] == 0


def test_repolist_all(utils):
    ret = utils.run(['tdnf', 'repolist', 'all'])
    assert ret['retval'] == 0


def test_repolist_enabled(utils):
    ret = utils.run(['tdnf', 'repolist', 'enabled'])
    assert ret['retval'] == 0


def test_repolist_disabled(utils):
    ret = utils.run(['tdnf', 'repolist', 'disabled'])
    assert ret['retval'] == 0


def test_repolist_invalid(utils):
    ret = utils.run(['tdnf', 'repolist', 'invalid_repo'])
    assert ret['retval'] == 901


# memcheck
def test_repolist_memcheck(utils):
    ret = utils.run_memcheck(['tdnf', 'repolist'])
    assert ret['retval'] == 0
