#
# Copyright (C) 2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#
#   Author: Oliver Kurth <okurth@vmware.com>

import os
import pytest
import time

REPOFILENAME = "photon-skip.repo"
REPOID = "photon-expire"


@pytest.fixture(scope='function', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    os.remove(os.path.join(utils.config['repo_path'], "yum.repos.d", REPOFILENAME))


def generate_repofile_expire(utils, newconfig, repoid, value):
    orig_repoconf = os.path.join(utils.config['repo_path'], "yum.repos.d", "photon-test.repo")
    option = "metadata_expire"

    with open(orig_repoconf, 'r') as fin:
        with open(newconfig, 'w') as fout:
            for line in fin:
                if line.startswith('['):
                    fout.write("[{}]\n".format(repoid))
                elif line.startswith('enabled'):
                    # we will enable this with the --repoid option
                    fout.write('enabled=0\n')
                elif not line.startswith(option):
                    fout.write(line)
            fout.write('{}={}\n'.format(option, value))


def test_cached_expired(utils):
    expire = 10
    repoconf = os.path.join(utils.config['repo_path'], "yum.repos.d", REPOFILENAME)
    generate_repofile_expire(utils, repoconf, REPOID, expire)

    utils.run(['tdnf', '--repoid={}'.format(REPOID), 'makecache'])
    time.sleep(expire + 2)
    ret = utils.run(['tdnf', '--repoid={}'.format(REPOID), 'list'])
    assert "Refreshing metadata" in "\n".join(ret['stdout'])


def test_cached_not_expired(utils):
    expire = 86400
    repoconf = os.path.join(utils.config['repo_path'], "yum.repos.d", REPOFILENAME)
    generate_repofile_expire(utils, repoconf, REPOID, expire)

    utils.run(['tdnf', '--repoid={}'.format(REPOID), 'makecache'])
    time.sleep(5)
    ret = utils.run(['tdnf', '--repoid={}'.format(REPOID), 'list'])
    assert "Refreshing metadata" not in "\n".join(ret['stdout'])


def test_cached_never_expired(utils):
    expire = "never"
    repoconf = os.path.join(utils.config['repo_path'], "yum.repos.d", REPOFILENAME)
    generate_repofile_expire(utils, repoconf, REPOID, expire)

    utils.run(['tdnf', '--repoid={}'.format(REPOID), 'makecache'])
    time.sleep(5)
    ret = utils.run(['tdnf', '--repoid={}'.format(REPOID), 'list'])
    assert "Refreshing metadata" not in "\n".join(ret['stdout'])


def test_cached_expired_cacheonly(utils):
    expire = 10
    repoconf = os.path.join(utils.config['repo_path'], "yum.repos.d", REPOFILENAME)
    generate_repofile_expire(utils, repoconf, REPOID, expire)

    utils.run(['tdnf', '--repoid={}'.format(REPOID), 'makecache'])
    time.sleep(expire + 2)
    ret = utils.run(['tdnf', '--repoid={}'.format(REPOID), '-C', 'list'])
    assert "Refreshing metadata" not in "\n".join(ret['stdout'])


# test for issue #302 - refresh marker was reset for any command
def test_cached_expired_no_reset(utils):
    expire = 10
    repoconf = os.path.join(utils.config['repo_path'], "yum.repos.d", REPOFILENAME)
    generate_repofile_expire(utils, repoconf, REPOID, expire)

    utils.run(['tdnf', '--repoid={}'.format(REPOID), 'makecache'])
    time.sleep(expire / 2)
    ret = utils.run(['tdnf', '--repoid={}'.format(REPOID), 'list'])
    time.sleep(expire / 2 + 2)
    ret = utils.run(['tdnf', '--repoid={}'.format(REPOID), 'list'])
    assert "Refreshing metadata" in "\n".join(ret['stdout'])
