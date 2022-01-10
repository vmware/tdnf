#
# Copyright (C) 2021-2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import os
import shutil
import pytest

REPODIR = '/root/repo_missing_metadata/yum.repos.d'
REPOFILENAME = 'repo_missing_metadata.repo'
REPONAME = "repo_missing_metadata-test"
WORKDIR = '/tmp/repo_missing_metadata/workdir'


@pytest.fixture(scope='function', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    if os.path.isdir(REPODIR):
        shutil.rmtree(REPODIR)
    if os.path.isdir(WORKDIR):
        shutil.rmtree(WORKDIR)


def test_repo_no_filelists(utils):
    reponame = 'photon-test'
    workdir = WORKDIR
    utils.makedirs(workdir)

    ret = utils.run(['tdnf', '--repo={}'.format(reponame),
                     '--download-metadata',
                     'reposync'],
                    cwd=workdir)
    assert(ret['retval'] == 0)
    synced_dir = os.path.join(workdir, reponame)
    assert(os.path.isdir(synced_dir))
    assert(os.path.isdir(os.path.join(synced_dir, 'repodata')))
    assert(os.path.isfile(os.path.join(synced_dir, 'repodata', 'repomd.xml')))

    for file in ['filelists', 'filelists_db']:
        ret = utils.run(['modifyrepo', '--remove', file, os.path.join(synced_dir, 'repodata')])
        assert(ret['retval'] == 0)

    ret = utils.run(['tdnf',
                     '--repofrompath=synced-repo,{}'.format(synced_dir),
                     '--repo=synced-repo',
                     'makecache'],
                    cwd=workdir)
    assert(ret['retval'] == 0)
