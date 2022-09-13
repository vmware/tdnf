#
# Copyright (C) 2019-2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import os
import tempfile
import pytest
import shutil
import platform

ARCH = platform.machine()


@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    pass


def test_check_local_no_args(utils):
    ret = utils.run(['tdnf', 'check-local'])
    assert ret['retval'] == 906


def test_check_local_with_invalid_dir(utils):
    ret = utils.run(['tdnf', 'check-local', '/home/invalid_dir'])
    assert ret['retval'] == 1602


def test_check_local_empty_directory(utils):
    temp_dir = tempfile.TemporaryDirectory()
    ret = utils.run(['tdnf', 'check-local', temp_dir.name])
    assert ret['retval'] == 0


def test_check_local_with_local_rpm(utils):
    with tempfile.TemporaryDirectory() as tmpdir:
        dest = os.path.join(tmpdir, 'test.rpm')
        src = os.path.join(utils.config['repo_path'], 'build', 'RPMS', ARCH, 'tdnf-test-two-1.0.1-1.{}.rpm'.format(ARCH))
        shutil.copyfile(src, dest)

        ret = utils.run(['tdnf', 'check-local', tmpdir])
        assert ret['retval'] == 0
