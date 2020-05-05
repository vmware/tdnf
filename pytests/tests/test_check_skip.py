#
# Copyright (C) 2019 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#
#   Author: Shreenidhi Shedi <sshedi@vmware.com>

import pytest


def run_cmd(utils, opt, retval):
    cmd = ['tdnf', 'check']
    cmd.extend(opt)
    ret = utils.run(cmd)
    assert(ret['retval'] == retval)


@pytest.fixture(scope='module', autouse = True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    pass


def test_check_skipconflicts(utils):
    run_cmd(utils, ['--skipconflicts'], 21)


def test_check_skipobsoletes(utils):
    run_cmd(utils, ['--skipobsoletes'], 21)


def test_check_providers(utils):
    run_cmd(utils, ['--skipconflicts', '--skipobsoletes'], 21)


def test_dummy_conflicts(utils):
    ret = utils.run(['tdnf', 'check', '--skipobsoletes'])
    assert ' conflicts ' in ret['stderr'][0]
    assert ' provides ' in ret['stderr'][1]


def test_dummy_obsoletes(utils):
    ret = utils.run(['tdnf', 'check', '--skipconflicts'])
    assert ' obsoletes ' in ret['stderr'][0]
    assert ' provides ' in ret['stderr'][1]


def test_dummy_provides(utils):
    ret = utils.run(['tdnf', 'check', '--skipconflicts', '--skipobsoletes'])
    assert ' provides ' in ret['stderr'][0]

def test_dummy_check(utils):
    ret = utils.run(['tdnf', 'check'])
    assert ' obsoletes ' in ret['stderr'][0]
    assert ' conflicts ' in ret['stderr'][1]
    assert ' provides ' in ret['stderr'][2]
