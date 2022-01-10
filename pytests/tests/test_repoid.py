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

REPODIR = '/root/repoid/yum.repos.d'
REPOFILENAME = 'repoid.repo'
REPONAME = "repoid-test"
WORKDIR = '/root/repoid/workdir'


@pytest.fixture(scope='function', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    if os.path.isdir(REPODIR):
        shutil.rmtree(REPODIR)
    if os.path.isdir(WORKDIR):
        shutil.rmtree(WORKDIR)
    filename = os.path.join(utils.config['repo_path'], "yum.repos.d", REPOFILENAME)
    if os.path.isfile(filename):
        os.remove(filename)


def test_repoid(utils):
    utils.makedirs(REPODIR)
    utils.create_repoconf(os.path.join(REPODIR, REPOFILENAME),
                          "http://foo.bar.com/packages",
                          REPONAME)
    ret = utils.run(['tdnf',
                     '--setopt=reposdir={}'.format(REPODIR),
                     '--repoid={}'.format(REPONAME),
                     'repolist'])
    assert(ret['retval'] == 0)
    assert(REPONAME in "\n".join(ret['stdout']))


# reposync a repo and install from it
def test_repoid_created_repo(utils):
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

    filename = os.path.join(utils.config['repo_path'], "yum.repos.d", REPOFILENAME)
    baseurl = "file://{}".format(synced_dir)

    utils.create_repoconf(filename, baseurl, "synced-repo")

    ret = utils.run(['tdnf',
                     '--repo=synced-repo',
                     'makecache'],
                    cwd=workdir)
    assert(ret['retval'] == 0)

    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)
    ret = utils.run(['tdnf',
                     '-y', '--nogpgcheck',
                     '--repo=synced-repo',
                     'install', pkgname],
                    cwd=workdir)
    assert(ret['retval'] == 0)
    assert(utils.check_package(pkgname))
