#
# Copyright (C) 2019-2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import os
import pytest

NEW_REPO_FN = 'test.repo'
ORIG_REPO_FN = 'photon-test.repo'


def create_repoconf(fn):
    content = '''
[cfg-test]
name=    Test Repo
baseurl = http://localhost:8080/photon-test
enabled      =1
gpgcheck=0
'''
    with open(fn, 'w') as f:
        f.write(content)


@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    global NEW_REPO_FN
    global ORIG_REPO_FN

    NEW_REPO_FN = os.path.join(utils.config['repo_path'], 'yum.repos.d',
                               NEW_REPO_FN)

    ORIG_REPO_FN = os.path.join(utils.config['repo_path'], 'yum.repos.d',
                                ORIG_REPO_FN)

    create_repoconf(NEW_REPO_FN)

    # take backup of original repo file
    os.rename(ORIG_REPO_FN, ORIG_REPO_FN + '.bak')

    yield

    teardown_test(utils)


def teardown_test(utils):
    os.remove(NEW_REPO_FN)
    os.rename(ORIG_REPO_FN + '.bak', ORIG_REPO_FN)
    ret = utils.run(['tdnf', 'makecache'])
    assert ret['retval'] == 0


def test_makecache(utils):
    ret = utils.run(['tdnf', 'makecache'])
    assert ret['retval'] == 0
    for i in ret['stdout']:
        if 'Metadata cache created' in i:
            return
    assert False


def test_repolist(utils):
    ret = utils.run(['tdnf', 'repolist'])
    assert ret['retval'] == 0
    for i in ret['stdout']:
        if 'cfg-test' in i and 'enabled' in i and 'Test Repo' in i:
            return
    assert False
