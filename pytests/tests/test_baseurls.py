#
# Copyright (C) 2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import os
import shutil
import pytest

WORKDIR = '/root/baseurls/workdir'
REPOFILENAME = 'baseurls.repo'
REPONAME = 'baseurls-repo'


@pytest.fixture(scope='function', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)
    if os.path.isdir(WORKDIR):
        shutil.rmtree(WORKDIR)
    filename = os.path.join(utils.config['repo_path'], "yum.repos.d", REPOFILENAME)
    if os.path.isfile(filename):
        os.remove(filename)


def test_multiple_baseurls(utils):
    reponame = REPONAME
    workdir = WORKDIR
    utils.makedirs(workdir)

    filename = os.path.join(utils.config['repo_path'], "yum.repos.d", REPOFILENAME)
    # we should get a 404 for the first url and skip to the next one
    baseurls = "http://localhost:8080/doesntexist http://localhost:8080/photon-test"
    utils.create_repoconf(filename, baseurls, reponame)

    ret = utils.run(['tdnf',
                     '--disablerepo=*', '--enablerepo={}'.format(reponame),
                     'makecache'],
                    cwd=workdir)
    assert ret['retval'] == 0

    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)
    ret = utils.run(['tdnf',
                     '-y', '--nogpgcheck',
                     '--disablerepo=*', '--enablerepo={}'.format(reponame),
                     'install', pkgname],
                    cwd=workdir)
    assert utils.check_package(pkgname)
