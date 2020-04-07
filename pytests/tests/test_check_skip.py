#
# Copyright (C) 2019 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#
#   Author: Shreenidhi Shedi <sshedi@vmware.com>

import os
import tempfile
import pytest
import shutil
import subprocess

isNotPhoton = -1
try:
    isNotPhoton = subprocess.check_call("uname -a | grep -i photon > /dev/null", shell = True)
except Exception as e:
    isNotPhoton = 1

def run_cmd(utils, cmd, retval):
    ret = utils.run(cmd)
    if retval == 0:
        assert(ret['retval'] == 0)
    else:
        assert(ret['retval'] != 0)

@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)

def teardown_test(utils):
    pass

def test_check_skipconflicts(utils):
    cmd = [ 'tdnf', '--config', '/etc/tdnf/tdnf.conf', 'check', '--skipconflicts']
    if isNotPhoton == 1:
        cmd.pop(1)
        cmd.pop(1)

    retval = 0 if isNotPhoton == 1 else 1
    run_cmd(utils, cmd, retval)

def test_check_skipobsoletes(utils):
    cmd = [ 'tdnf', '--config', '/etc/tdnf/tdnf.conf', 'check', '--skipobsoletes']
    if isNotPhoton == 1:
        cmd.pop(1)
        cmd.pop(1)

    retval = 0 if isNotPhoton == 1 else 1
    run_cmd(utils, cmd, retval)

def test_check_providers(utils):
    cmd = [ 'tdnf', '--config', '/etc/tdnf/tdnf.conf', 'check', '--skipconflicts', '--skipobsoletes']
    if isNotPhoton == 1:
        cmd.pop(1)
        cmd.pop(1)

    retval = 1 if isNotPhoton == 1 else 0
    run_cmd(utils, cmd, 0)
