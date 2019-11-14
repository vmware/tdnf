#            (c) 2019 VMware Inc.,
#
#   Author: Siddharth Chandrasekaran <csiddharth@vmware.com>

import os
import tempfile
import pytest

@pytest.fixture(scope='module', autouse=True)
def setup_test_cache(utils):
    yield
    teardown_test_cache(utils)

def teardown_test_cache(utils):
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
        url = utils.config["check_local_pkg_url"]
        utils.wget(url, dest, enforce_https=False)
        ret = utils.run([ 'tdnf', 'check_local', tmpdir ])
        assert (ret['retval'] == 0)
