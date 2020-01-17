#
# Copyright (C) 2019 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#
#   Author: Siddharth Chandrasekaran <csiddharth@vmware.com>

import os
import tempfile
import pytest
import shutil

@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)

def teardown_test(utils):
    pass

def test_check_local_no_args(utils):
    ret = utils.run([ 'tdnf', 'check_local' ])
    assert (ret['retval'] == 0)

def test_check_local_empty_directory(utils):
    temp_dir = tempfile.TemporaryDirectory()
    ret = utils.run([ 'tdnf', 'check_local', temp_dir.name ])
    assert (ret['retval'] == 0)

def test_check_local_with_one_rpm(utils):
    with tempfile.TemporaryDirectory() as tmpdir:
        dest = os.path.join(tmpdir, 'test.rpm')
        src = os.path.join(utils.config['repo_path'], 'build/RPMS/x86_64/tdnf-test-two-1.0.1-1.x86_64.rpm')
        shutil.copyfile(src, dest)

        ret = utils.run([ 'tdnf', 'check_local', tmpdir ])
        assert (ret['retval'] == 0)
