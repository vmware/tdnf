#
# Copyright (C) 2026 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

# Test that a package excluded from a snapshot cannot be pulled in as an
# obsoleting replacement when installing the package it obsoletes.
#
# Scenario:
#   tdnf-test-dummy-obsoleting  Obsoletes/Provides: tdnf-test-dummy-obsoleted
#   tdnf-test-dummy-obsoleted   (the package being obsoleted)
#
# The snapshot includes tdnf-test-dummy-obsoleted but EXCLUDES
# tdnf-test-dummy-obsoleting.  Installing tdnf-test-dummy-obsoleted through
# the snapshot repo must NOT silently install tdnf-test-dummy-obsoleting in
# its place.

import os
import pytest

REPOFILENAME = "snapshot-obsoletes.repo"
REPONAME = "snapshot-obsoletes"

PKGNAME_OBSOLETED = "tdnf-test-dummy-obsoleted"
PKGNAME_OBSOLETING = "tdnf-test-dummy-obsoleting"


def create_snapshot_repo(utils, reponame):
    """
    Build a snapshot list that includes tdnf-test-dummy-obsoleted but
    explicitly omits tdnf-test-dummy-obsoleting.
    """
    snapshot_file = os.path.join(
        utils.config['repo_path'], "yum.repos.d", f"{reponame}.list"
    )

    ret = utils.run(["tdnf", "repoquery", "--available", "--qf", "%{name}=%{evr}"])
    snapshot_list = ret['stdout']

    with open(snapshot_file, "wt") as f:
        for pkg in snapshot_list:
            # Exclude the obsoleting package from the snapshot.
            if not pkg.startswith(PKGNAME_OBSOLETING):
                f.write(f"{pkg}\n")

    baseurls = "http://localhost:8080/photon-test"
    utils.create_repoconf(
        os.path.join(utils.config['repo_path'], "yum.repos.d", REPOFILENAME),
        baseurls,
        reponame,
    )
    utils.edit_config({'snapshot': f"{reponame}.list"}, repo=REPONAME)


@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    create_snapshot_repo(utils, REPONAME)
    yield
    teardown_test(utils)


def teardown_test(utils):
    for filename in [
        os.path.join(utils.config['repo_path'], "yum.repos.d", REPOFILENAME),
        os.path.join(utils.config['repo_path'], "yum.repos.d", f"{REPONAME}.list"),
    ]:
        try:
            os.remove(filename)
        except OSError:
            pass

    utils.erase_package(PKGNAME_OBSOLETED)
    utils.erase_package(PKGNAME_OBSOLETING)


def test_obsoleting_pkg_not_in_snapshot(utils):
    """
    The obsoleting package must not appear in the snapshot repo's available
    list, confirming the snapshot filter is in effect.
    """
    import json
    ret = utils.run(["tdnf", "-j", "--repoid", REPONAME, "--available", "list"])
    infolist = json.loads("\n".join(ret['stdout']))

    names = {info['Name'] for info in infolist}
    assert PKGNAME_OBSOLETING not in names, (
        f"{PKGNAME_OBSOLETING} should be excluded from the snapshot repo"
    )
    assert PKGNAME_OBSOLETED in names, (
        f"{PKGNAME_OBSOLETED} should be present in the snapshot repo"
    )


def test_install_obsoleted_via_snapshot_does_not_pull_obsoleting(utils):
    """
    Installing the obsoleted package through the snapshot repo must install
    exactly that package.  The obsoleting package is outside the snapshot and
    must NOT be installed in its place.
    """
    utils.erase_package(PKGNAME_OBSOLETED)
    utils.erase_package(PKGNAME_OBSOLETING)

    ret = utils.run([
        "tdnf", "-y", "--nogpgcheck",
        "--disablerepo=*", "--repoid", REPONAME,
        "install", PKGNAME_OBSOLETED,
    ])
    assert ret['retval'] == 0, (
        f"install of {PKGNAME_OBSOLETED} via snapshot repo failed: {ret['stderr']}"
    )

    # The obsoleted package (or its exact-version equivalent) must be present.
    assert utils.check_package(PKGNAME_OBSOLETED), (
        f"{PKGNAME_OBSOLETED} was not installed"
    )

    # The obsoleting package must NOT have been pulled in.
    assert not utils.check_package(PKGNAME_OBSOLETING), (
        f"{PKGNAME_OBSOLETING} was incorrectly installed via the snapshot repo "
        f"even though it is excluded from the snapshot"
    )
