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

WORKDIR = '/root/mirrortest/workdir'
REPONAME = 'mirrortest'
REPOFILENAME = f"{REPONAME}.repo"
MIRRORLIST_FILENAME = "mirror.list"


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


def create_repo_conf(repos, reposdir="/etc/yum.repos.d", insecure=False):
    """
    Create .repo file as per configurations passed.
    Parameters:
    - repos: Dictionary containing repo_id as key and value as dictionary containing repo configurations.
             Ex: {'repo_id1': {'baseurl': 'https://foo/bar', 'enabled': 0}, 'repo_id2': {'basurl': '/mnt/media/RPMS', 'enabled': 1}}
    - reposdir (Optional): Parent dir where .repo needs to be placed. Default Value - /etc/yum.repos.d/{repo_name}.repo
    Returns:
    - None
    """
    os.makedirs(reposdir, exist_ok=True)
    for repo in repos:
        if insecure:
            repos[repo]["sslverify"] = 0
        with open(os.path.join(reposdir, f"{repo}.repo"), "w", encoding="utf-8") as repo_file:
            repo_file.write(f"[{repo}]\n")
            for key, value in repos[repo].items():
                repo_file.write(f"{key}={value}\n")


def test_mirrrorlist(utils):
    reponame = REPONAME
    workdir = WORKDIR
    utils.makedirs(workdir)
    mirrorlist_path = os.path.join(workdir, MIRRORLIST_FILENAME)

    # we should get a 404 for the first url and skip to the next one
    baseurls = ["http://localhost:8080/doesntexist", "http://localhost:8080/photon-test"]
    with open(mirrorlist_path, "wt") as f:
        for url in baseurls:
            f.write(f"{url}\n")

    create_repo_conf({reponame: {'enabled': 1, 'gpgcheck:': 0, 'mirrorlist': f"file://{mirrorlist_path}"}},
                     os.path.join(utils.config['repo_path'], "yum.repos.d"),
                     True)

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
