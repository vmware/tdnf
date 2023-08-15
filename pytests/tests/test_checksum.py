# Copyright (C) 2019 - 2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import os
import shutil
import filecmp
import glob
import pytest

WORKDIR = '/root/repofrompath/workdir'


@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    utils.run(['tdnf', 'erase', '-y', 'tdnf-test-one'])
    if os.path.isdir(WORKDIR):
        shutil.rmtree(WORKDIR)


def enable_and_create_repo(utils):
    workdir = WORKDIR
    utils.makedirs(workdir)
    reponame = 'photon-test-sha512'

    ret = utils.run(['tdnf', '-v', '--disablerepo=*', '--enablerepo=photon-test-sha512', 'makecache'])
    assert ret['retval'] == 0

    ret = utils.run(['tdnf', '--repo={}'.format(reponame),
                     '--download-metadata',
                     'reposync'],
                    cwd=workdir)
    assert ret['retval'] == 0
    synced_dir = os.path.join(workdir, reponame)
    assert os.path.isdir(synced_dir)
    assert os.path.isdir(os.path.join(synced_dir, 'repodata'))
    assert os.path.isfile(os.path.join(synced_dir, 'repodata', 'repomd.xml'))


def copy_rpm(workdir, orig_pkg, copy_pkg):

    orig_path = glob.glob(f"{workdir}/{orig_pkg}*.rpm")
    copy_path = glob.glob(f"{workdir}/{copy_pkg}*.rpm")

    if not filecmp.cmp(copy_path[0], orig_path[0]):
        shutil.copy2(copy_path[0], orig_path[0])


# install package with SHA512
def test_install_package_with_sha512_checksum(utils):
    ret = utils.run(['tdnf', 'erase', '-y', 'tdnf-test-one'])
    assert ret['retval'] == 0

    ret = utils.run(['tdnf', '-y', '--nogpgcheck', 'install', '-y', 'tdnf-test-one', '--enablerepo=photon-test-sha512'])
    assert ret['retval'] == 0


# install package with incorrect SHA512
def test_install_package_with_incorrect_sha512_checksum(utils):
    workdir = WORKDIR
    reponame = 'photon-test-sha512'
    synced_dir = os.path.join(workdir, reponame)
    rpm_dir = os.path.join(synced_dir, 'RPMS')
    rpm_dir = os.path.join(rpm_dir, 'x86_64')

    enable_and_create_repo(utils)
    copy_rpm(rpm_dir, 'tdnf-test-one', 'tdnf-test-two')

    ret = utils.run(['tdnf',
                     '--repofrompath=synced-repo,{}'.format(synced_dir),
                     '--repo=synced-repo',
                     'makecache'],
                    cwd=workdir)
    assert ret['retval'] == 0

    ret = utils.run(['tdnf', 'erase', '-y', 'tdnf-test-one'], cwd=workdir)
    assert ret['retval'] == 0

    ret = utils.run(['tdnf', '-y', '--nogpgcheck', '--repofrompath=synced-repo,{}'.format(synced_dir), '--repo=synced-repo', 'install', '-y', 'tdnf-test-one'], cwd=workdir)
    assert ret['retval'] == 1528
