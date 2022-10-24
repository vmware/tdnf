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


def test_skipobsoletes(utils):
    ret = utils.run(['tdnf', 'check', '--skipobsoletes'])
    assert ret['retval'] == 1301
    assert ' conflicts ' in '\n'.join(ret['stderr'])
    assert ' provides ' in '\n'.join(ret['stderr'])
    assert ' obsoletes ' not in '\n'.join(ret['stderr'])


def test_skipconflicts(utils):
    ret = utils.run(['tdnf', 'check', '--skipconflicts'])
    assert ret['retval'] == 1301
    assert ' obsoletes ' in '\n'.join(ret['stderr'])
    assert ' provides ' in '\n'.join(ret['stderr'])
    assert ' conflicts ' not in '\n'.join(ret['stderr'])


def test_skipconflicts_skipobsoletes(utils):
    ret = utils.run(['tdnf', 'check', '--skipconflicts', '--skipobsoletes'])
    assert ret['retval'] == 1301
    assert ' provides ' in '\n'.join(ret['stderr'])
    assert ' conflicts ' not in '\n'.join(ret['stderr'])
    assert ' obsoletes ' not in '\n'.join(ret['stderr'])


def test_check_no_skip(utils):
    ret = utils.run(['tdnf', 'check'])
    assert ' obsoletes ' in '\n'.join(ret['stderr'])
    assert ' conflicts ' in '\n'.join(ret['stderr'])
    assert ' provides ' in '\n'.join(ret['stderr'])
