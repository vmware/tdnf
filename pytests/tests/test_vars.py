#
# Copyright (C) 2024 Broadcom, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import os
import shutil
import pytest

WORKDIR = '/root/vars/workdir'
REPOFILENAME = 'baseurls.repo'
REPONAME = 'baseurls-repo'


@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    workdir = WORKDIR
    utils.makedirs(workdir)
    utils.edit_config({'varsdir': workdir})

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


def test_vars(utils):
    reponame = REPONAME
    workdir = WORKDIR

    varsdir = workdir
    port_var = "8080"
    dir_var = "photon-test"

    with open(os.path.join(varsdir, "port"), "wt") as f:
        f.write(port_var + "\n")

    with open(os.path.join(varsdir, "dir"), "wt") as f:
        f.write(dir_var + "\n")

    filename = os.path.join(utils.config['repo_path'], "yum.repos.d", REPOFILENAME)

    baseurls = "http://localhost:$port/$dir"
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
