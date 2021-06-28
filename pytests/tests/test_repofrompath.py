#
# Copyright (C) 2021 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#
#   Author: Oliver Kurth <okurth@vmware.com>

import os
import shutil
import pytest
import errno

REPODIR='/root/repofrompath/yum.repos.d'
REPOFILENAME='repofrompath.repo'
REPONAME="repofrompath-test"
WORKDIR='/root/repofrompath/workdir'

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

# reposync a repo and install from it
def test_repofrompath_created_repo(utils):
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

    baseurl = "file://{}".format(synced_dir)

    ret = utils.run(['tdnf',
                     '--repofrompath=synced-repo,{}'.format(synced_dir),
                     '--repo=synced-repo',
                     'makecache'],
                    cwd=workdir)
    assert(ret['retval'] == 0)

    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)
    ret = utils.run(['tdnf',
                     '-y', '--nogpgcheck',
                     '--repofrompath=synced-repo,{}'.format(synced_dir),
                     '--repo=synced-repo',
                     'install', pkgname ],
                    cwd=workdir)
    assert(ret['retval'] == 0)
    assert(utils.check_package(pkgname) == True)

