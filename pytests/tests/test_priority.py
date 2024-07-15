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

REPODIR = "/root/priority/repo"
REPONAME = "priority-test"
REPOFILENAME = f"{REPONAME}.repo"


@pytest.fixture(scope='function', autouse=True)
def setup_test(utils):

    os.makedirs(REPODIR, exist_ok=True)

    yield
    teardown_test(utils)


def teardown_test(utils):
    if os.path.isdir(REPODIR):
        shutil.rmtree(REPODIR)
    filename = os.path.join(utils.config['repo_path'], "yum.repos.d", REPOFILENAME)
    if os.path.isfile(filename):
        os.remove(filename)


def test_priority(utils):
    pkgname = utils.config['mulversion_pkgname']
    pkgname_low = pkgname + "=" + utils.config['mulversion_lower']
    ret = utils.run(["tdnf",
                     "-y", "--nogpgcheck",
                     "--downloadonly", f"--downloaddir={REPODIR}",
                     "install", pkgname_low],
                    )
    assert ret['retval'] == 0

    ret = utils.run(["createrepo", "."], cwd=REPODIR)
    assert ret['retval'] == 0

    filename = os.path.join(utils.config['repo_path'], "yum.repos.d", REPOFILENAME)
    baseurl = "file://{}".format(REPODIR)

    utils.create_repoconf(filename, baseurl, REPONAME)
    utils.edit_config({'priority': "25"}, repo=REPONAME)

    ret = utils.run(["tdnf",
                     "-y", "--nogpgcheck",
                     "install", pkgname],
                    cwd=REPODIR)
    assert ret['retval'] == 0

    utils.check_package(pkgname, version=utils.config['mulversion_lower'])
