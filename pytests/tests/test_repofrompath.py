#
# Copyright (C) 2021-2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import os
import glob
import shutil
import platform
import pytest

WORKDIR = '/root/repofrompath/workdir'
ARCH = platform.machine()


@pytest.fixture(scope='function', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    if os.path.isdir(WORKDIR):
        shutil.rmtree(WORKDIR)


def create_repo(utils):
    workdir = WORKDIR
    utils.makedirs(workdir)
    reponame = 'photon-test'

    ret = utils.run(['tdnf', '--repo={}'.format(reponame),
                     '--download-metadata',
                     'reposync'],
                    cwd=workdir)
    assert ret['retval'] == 0
    synced_dir = os.path.join(workdir, reponame)
    assert os.path.isdir(synced_dir)
    assert os.path.isdir(os.path.join(synced_dir, 'repodata'))
    assert os.path.isfile(os.path.join(synced_dir, 'repodata', 'repomd.xml'))


def get_pkg_file_path(utils, pkgname):
    dir = os.path.join(utils.config['repo_path'], 'photon-test', 'RPMS', ARCH)
    matches = glob.glob('{}/{}-*.rpm'.format(dir, pkgname))
    return matches[0]


# reposync a repo and install from it
def test_repofrompath_created_repo(utils):
    workdir = WORKDIR
    reponame = 'photon-test'
    synced_dir = os.path.join(workdir, reponame)

    create_repo(utils)

    ret = utils.run(['tdnf',
                     '--repofrompath=synced-repo,{}'.format(synced_dir),
                     '--repo=synced-repo',
                     'makecache'],
                    cwd=workdir)
    assert ret['retval'] == 0

    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)
    ret = utils.run(['tdnf',
                     '-y', '--nogpgcheck',
                     '--repofrompath=synced-repo,{}'.format(synced_dir),
                     '--repo=synced-repo',
                     'install', pkgname],
                    cwd=workdir)
    assert ret['retval'] == 0
    assert utils.check_package(pkgname)


# check for issue #359 - having set a repo with --repofrompath
# should not interfer with installing a package from the command line
def test_repofrompath_cmdline_repo(utils):
    workdir = WORKDIR
    reponame = 'photon-test'
    synced_dir = os.path.join(workdir, reponame)

    create_repo(utils)

    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)

    path = get_pkg_file_path(utils, pkgname)

    ret = utils.run(['tdnf',
                     '-y', '--nogpgcheck',
                     '--repofrompath=synced-repo,{}'.format(synced_dir),
                     '--repo=synced-repo',
                     'install', path],
                    cwd=workdir)
    assert ret['retval'] == 0
    assert utils.check_package(pkgname)


# reposync a repo and install from it using repofromdir
def test_repofromdir_created_repo(utils):
    reponame = 'photon-test'
    workdir = WORKDIR
    utils.makedirs(workdir)

    ret = utils.run(['tdnf', '--repo={}'.format(reponame),
                     'reposync'],
                    cwd=workdir)
    assert ret['retval'] == 0
    synced_dir = os.path.join(workdir, reponame)
    assert os.path.isdir(synced_dir)
    # repofromdir must work without repodata
    assert not os.path.isdir(os.path.join(synced_dir, 'repodata'))

    ret = utils.run(['tdnf',
                     '--repofromdir=synced-repo,{}'.format(synced_dir),
                     '--repo=synced-repo',
                     'makecache'],
                    cwd=workdir)
    assert ret['retval'] == 0

    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)
    ret = utils.run(['tdnf',
                     '-y', '--nogpgcheck',
                     '--repofromdir=synced-repo,{}'.format(synced_dir),
                     '--repo=synced-repo',
                     'install', pkgname],
                    cwd=workdir)
    assert ret['retval'] == 0
    assert utils.check_package(pkgname)
