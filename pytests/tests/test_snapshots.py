#
# Copyright (C) 2025 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import json
import pytest
import os


REPOFILENAME = "snapshot.repo"
REPONAME = "snapshot"

EXCLUDED_PKGS = ["tdnf-multi=1.0.1-1", "tdnf-multi=1.0.1-2", "tdnf-multi=1.0.1-4"]
INCLUDED_PKGS = ["tdnf-multi=1.0.1-3"]


def create_snapshot_repo(utils, reponame):
    snapshot_file = os.path.join(utils.config['repo_path'], "yum.repos.d", f"{reponame}.list")

    ret = utils.run(["tdnf", "repoquery", "--available", "--qf", "%{name}=%{evr}"])
    snapshot_list = ret['stdout']

    with open(snapshot_file, "wt") as f:
        for pkg in snapshot_list:
            if pkg not in EXCLUDED_PKGS:
                f.write(f"{pkg}\n")

    baseurls = "http://localhost:8080/photon-test"
    utils.create_repoconf(os.path.join(utils.config['repo_path'], "yum.repos.d", REPOFILENAME), baseurls, reponame)
    utils.edit_config({'snapshot': f"{reponame}.list"}, repo=REPONAME)


@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    create_snapshot_repo(utils, REPONAME)
    yield
    teardown_test(utils)


def teardown_test(utils):
    for filename in [os.path.join(utils.config['repo_path'], "yum.repos.d", REPOFILENAME), os.path.join(utils.config['repo_path'], "yum.repos.d", f"{REPONAME}.list")]:
        try:
            os.remove(filename)
        except OSError:
            pass

    for pkg in EXCLUDED_PKGS + INCLUDED_PKGS:
        utils.erase_package(pkg)


def test_install(utils):

    # expect failure to install excluded package
    for pkg in EXCLUDED_PKGS:
        ret = utils.run(["tdnf", "-y", "install", "--disablerepo=*", "--repoid", REPONAME, pkg])
        assert ret['retval'] != 0

    # expect succes to install included package
    for pkg in INCLUDED_PKGS:
        ret = utils.run(["tdnf", "-y", "install", "--repoid", REPONAME, pkg])
        assert ret['retval'] == 0


def test_update(utils):
    # clean up before test
    for pkg in EXCLUDED_PKGS + INCLUDED_PKGS:
        utils.erase_package(pkg)

    # install from non-snapshot repo
    ret = utils.run(["tdnf", "-y", "install", "tdnf-multi=1.0.1-1"])
    assert ret['retval'] == 0

    ret = utils.run(["tdnf", "-y", "--repoid", REPONAME, "update", "tdnf-multi"])
    assert ret['retval'] == 0

    ret = utils.run(["tdnf", "-j", "--installed", "list"])
    infolist = json.loads("\n".join(ret['stdout']))
    print(f"infolist={infolist}\n")

    found = False
    for info in infolist:
        nevr = f"{info['Name']}={info['Evr']}"
        print(f"nevr={nevr}\n")
        if nevr in INCLUDED_PKGS:
            found = True
        assert nevr not in EXCLUDED_PKGS

    assert found


def test_list(utils):
    ret = utils.run(["tdnf", "-j", "--repoid", REPONAME, "--available", "list"])
    infolist = json.loads("\n".join(ret['stdout']))

    # excluded packages should not be listed
    for info in infolist:
        nevr = f"{info['Name']}={info['Evr']}"
        assert nevr not in EXCLUDED_PKGS


def test_list_file_absolute(utils):
    snapshot_file = os.path.join(utils.config['repo_path'], "yum.repos.d", f"{REPONAME}.list")

    ret = utils.run(["tdnf", "-j", "--repoid", REPONAME, "--available", f"--setopt=snapshot.{REPONAME}={snapshot_file}", "list"])
    infolist = json.loads("\n".join(ret['stdout']))

    # excluded packages should not be listed
    for info in infolist:
        nevr = f"{info['Name']}={info['Evr']}"
        assert nevr not in EXCLUDED_PKGS


def test_list_file_notfound(utils):
    snapshot_file = os.path.join(utils.config['repo_path'], "yum.repos.d", f"{REPONAME}.list.invalid")

    ret = utils.run(["tdnf", "-j", "--repoid", REPONAME, "--available", f"--setopt=snapshot.{REPONAME}={snapshot_file}", "list"])
    assert ret['retval'] != 0


def test_list_file_url(utils):
    snapshot_file = os.path.join(utils.config['repo_path'], "yum.repos.d", f"{REPONAME}.list")

    ret = utils.run(["tdnf", "-j", "--repoid", REPONAME, "--available", f"--setopt=snapshot.{REPONAME}=file://{snapshot_file}", "list"])
    infolist = json.loads("\n".join(ret['stdout']))

    # excluded packages should not be listed
    for info in infolist:
        nevr = f"{info['Name']}={info['Evr']}"
        assert nevr not in EXCLUDED_PKGS


def test_list_http(utils):
    snapshot_url = f"http://localhost:8080/yum.repos.d/{REPONAME}.list"

    ret = utils.run(["tdnf", "-j", "--repoid", REPONAME, "--available", f"--setopt=snapshot.{REPONAME}={snapshot_url}", "list"])
    infolist = json.loads("\n".join(ret['stdout']))

    # excluded packages should not be listed
    for info in infolist:
        nevr = f"{info['Name']}={info['Evr']}"
        assert nevr not in EXCLUDED_PKGS


def test_list_http_404(utils):
    snapshot_url = f"http://localhost:8080/yum.repos.d/{REPONAME}.list.invalid"

    ret = utils.run(["tdnf", "-j", "--repoid", REPONAME, "--available", f"--setopt=snapshot.{REPONAME}={snapshot_url}", "list"])
    assert ret['retval'] != 0


def test_info(utils):
    ret = utils.run(["tdnf", "-j", "--repoid", REPONAME, "--available", "info"])
    infolist = json.loads("\n".join(ret['stdout']))

    # excluded packages should not be listed
    for info in infolist:
        nevr = f"{info['Name']}={info['Evr']}"
        assert nevr not in EXCLUDED_PKGS


def test_repoquery(utils):
    ret = utils.run(["tdnf", "-j", "--repoid", REPONAME, "--available", "repoquery"])
    infolist = json.loads("\n".join(ret['stdout']))

    # excluded packages should not be listed
    for info in infolist:
        nevr = f"{info['Name']}={info['Evr']}"
        assert nevr not in EXCLUDED_PKGS
