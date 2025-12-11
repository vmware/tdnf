#
# Copyright (C) 2021-2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import os
import shutil
import json
import pytest

REPODIR = '/root/repoid/yum.repos.d'
REPOFILENAME = 'repoid.repo'
REPONAME = "repoid-test"
WORKDIR = '/root/repoid/workdir'
BASEDIR = os.path.dirname(WORKDIR)


@pytest.fixture(scope='function', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    if os.path.isdir(BASEDIR):
        shutil.rmtree(BASEDIR)
    filename = os.path.join(utils.config['repo_path'], "yum.repos.d", REPOFILENAME)
    if os.path.isfile(filename):
        os.remove(filename)


def test_repoid(utils):
    os.makedirs(REPODIR, exist_ok=True)
    utils.create_repoconf(os.path.join(REPODIR, REPOFILENAME),
                          "http://foo.bar.com/packages",
                          REPONAME)
    ret = utils.run(['tdnf',
                     '--setopt=reposdir={}'.format(REPODIR),
                     '--repoid={}'.format(REPONAME),
                     'repolist'])
    assert ret['retval'] == 0
    assert REPONAME in "\n".join(ret['stdout'])


def test_repo(utils):
    os.makedirs(REPODIR, exist_ok=True)
    utils.create_repoconf(os.path.join(REPODIR, REPOFILENAME),
                          "http://foo.bar.com/packages",
                          REPONAME)
    ret = utils.run(['tdnf',
                     '--setopt=reposdir={}'.format(REPODIR),
                     '--repo={}'.format(REPONAME),
                     'repolist'])
    assert ret['retval'] == 0
    assert REPONAME in "\n".join(ret['stdout'])


# reposync a repo and install from it
def test_repoid_created_repo(utils):
    reponame = 'photon-test'
    workdir = WORKDIR
    os.makedirs(workdir, exist_ok=True)

    ret = utils.run(['tdnf', '--repo={}'.format(reponame),
                     '--download-metadata',
                     'reposync'],
                    cwd=workdir)
    assert ret['retval'] == 0
    synced_dir = os.path.join(workdir, reponame)
    assert os.path.isdir(synced_dir)
    assert os.path.isdir(os.path.join(synced_dir, 'repodata'))
    assert os.path.isfile(os.path.join(synced_dir, 'repodata', 'repomd.xml'))

    filename = os.path.join(utils.config['repo_path'], "yum.repos.d", REPOFILENAME)
    baseurl = "file://{}".format(synced_dir)

    utils.create_repoconf(filename, baseurl, "synced-repo")

    ret = utils.run(['tdnf',
                     '--repo=synced-repo',
                     'makecache'],
                    cwd=workdir)
    assert ret['retval'] == 0

    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)
    ret = utils.run(['tdnf',
                     '-y', '--nogpgcheck',
                     '--repo=synced-repo',
                     'install', pkgname],
                    cwd=workdir)
    assert ret['retval'] == 0
    assert utils.check_package(pkgname)
    utils.erase_package(pkgname)


def find_repo(repolist, id):
    for repo in repolist:
        if repo['Repo'] == id:
            return True
    return False


# Test comma-separated repoid
def test_repoid_comma_separated(utils):
    os.makedirs(REPODIR, exist_ok=True)

    # Create first repo
    REPONAME1 = "repoid-test1"
    utils.create_repoconf(os.path.join(REPODIR, 'repoid1.repo'),
                          "http://foo.bar.com/packages1",
                          REPONAME1)

    # Create second repo
    REPONAME2 = "repoid-test2"
    utils.create_repoconf(os.path.join(REPODIR, 'repoid2.repo'),
                          "http://foo.bar.com/packages2",
                          REPONAME2)

    # Create third repo (should be disabled when using --repoid)
    REPONAME3 = "repoid-test3"
    utils.create_repoconf(os.path.join(REPODIR, 'repoid3.repo'),
                          "http://foo.bar.com/packages3",
                          REPONAME3)

    # Test with comma-separated repoid - should only enable the specified repos
    ret = utils.run(['tdnf',
                     '--setopt=reposdir={}'.format(REPODIR),
                     '--repoid={},{}'.format(REPONAME1, REPONAME2),
                     'repolist', '-j'])
    assert ret['retval'] == 0
    repolist = json.loads("\n".join(ret['stdout']))

    # Verify that the specified repos are enabled
    assert find_repo(repolist, REPONAME1)
    assert find_repo(repolist, REPONAME2)

    # Verify that the third repo is NOT enabled (repoid disables all others)
    assert not find_repo(repolist, REPONAME3)


# Test comma-separated repo (alias for repoid)
def test_repo_comma_separated(utils):
    os.makedirs(REPODIR, exist_ok=True)

    # Create first repo
    REPONAME1 = "repo-test1"
    utils.create_repoconf(os.path.join(REPODIR, 'repo1.repo'),
                          "http://foo.bar.com/packages1",
                          REPONAME1)

    # Create second repo
    REPONAME2 = "repo-test2"
    utils.create_repoconf(os.path.join(REPODIR, 'repo2.repo'),
                          "http://foo.bar.com/packages2",
                          REPONAME2)

    # Create third repo (should be disabled when using --repo)
    REPONAME3 = "repo-test3"
    utils.create_repoconf(os.path.join(REPODIR, 'repo3.repo'),
                          "http://foo.bar.com/packages3",
                          REPONAME3)

    # Test with comma-separated repo - should only enable the specified repos
    ret = utils.run(['tdnf',
                     '--setopt=reposdir={}'.format(REPODIR),
                     '--repo={},{}'.format(REPONAME1, REPONAME2),
                     'repolist', '-j'])
    assert ret['retval'] == 0
    repolist = json.loads("\n".join(ret['stdout']))

    # Verify that the specified repos are enabled
    assert find_repo(repolist, REPONAME1)
    assert find_repo(repolist, REPONAME2)

    # Verify that the third repo is NOT enabled (repo disables all others)
    assert not find_repo(repolist, REPONAME3)
