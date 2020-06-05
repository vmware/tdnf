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
    assert(ret['retval'] == 0)


@pytest.fixture(scope='module', autouse = True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    pass


def test_check_skipconflicts(utils):
    run_cmd(utils, ['--skipconflicts'], 0)


def test_check_skipobsoletes(utils):
    run_cmd(utils, ['--skipobsoletes'], 0)


def test_check_providers(utils):
    run_cmd(utils, ['--skipconflicts', '--skipobsoletes'], 0)
