#
# Copyright (C) 2024 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import os
import glob
import shutil
import pytest


def test_download_dryrun_preserves_destination(utils, tmp_path):
    pkgname = utils.config['sglversion_pkgname']
    destination = tmp_path / 'downloads'
    args = ['tdnf', 'download', '-y', '--nogpgcheck',
            '--downloaddir', str(destination), pkgname]

    ret = utils.run(args + ['--dryrun'])
    assert ret['retval'] == 0
    assert not destination.exists()
    assert not any(line.startswith('Downloaded ') for line in ret['stdout'])

    ret = utils.run(args)
    assert ret['retval'] == 0
    assert list((destination / pkgname).glob('*.rpm'))


DOWNLOADBASE = '/tmp/tdnf/download-test'


@pytest.fixture(scope='function', autouse=True)
def setup_test_function(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    pkgname = utils.config["sglversion_pkgname"]
    utils.erase_package(pkgname)
    if os.path.isdir(DOWNLOADBASE):
        shutil.rmtree(DOWNLOADBASE)


# download with no package name should fail
def test_download_no_args(utils):
    ret = utils.run(['tdnf', 'download'])
    assert ret['retval'] != 0


# download should fail on a non-existent package
def test_download_invalid_package(utils):
    ret = utils.run(['tdnf', 'download', '-y', 'this-package-does-not-exist'])
    assert ret['retval'] != 0


# download should not install the package
def test_download_does_not_install(utils):
    pkgname = utils.config["sglversion_pkgname"]
    utils.erase_package(pkgname)
    os.makedirs(DOWNLOADBASE, exist_ok=True)

    ret = utils.run(['tdnf', 'download', '-y', '--nogpgcheck',
                     '--downloaddir', DOWNLOADBASE, pkgname])
    assert ret['retval'] == 0
    assert not utils.check_package(pkgname)


# per-package subdir is created under the current working directory
def test_download_creates_subdir_in_cwd(utils):
    pkgname = utils.config["sglversion_pkgname"]
    utils.erase_package(pkgname)

    tmpdir = '/tmp/tdnf-cwd-test'
    os.makedirs(tmpdir, exist_ok=True)
    try:
        orig_dir = os.getcwd()
        os.chdir(tmpdir)
        try:
            ret = utils.run(['tdnf', 'download', '-y', '--nogpgcheck', pkgname])
            assert ret['retval'] == 0
            pkgdir = os.path.join(tmpdir, pkgname)
            assert os.path.isdir(pkgdir)
            assert len(glob.glob('{}/*.rpm'.format(pkgdir))) > 0
        finally:
            os.chdir(orig_dir)
    finally:
        shutil.rmtree(tmpdir, ignore_errors=True)


# per-package subdir is created under --downloaddir
def test_download_to_directory(utils):
    pkgname = utils.config["sglversion_pkgname"]
    utils.erase_package(pkgname)
    os.makedirs(DOWNLOADBASE, exist_ok=True)

    ret = utils.run(['tdnf', 'download', '-y', '--nogpgcheck',
                     '--downloaddir', DOWNLOADBASE, pkgname])
    assert ret['retval'] == 0

    pkgdir = os.path.join(DOWNLOADBASE, pkgname)
    assert os.path.isdir(pkgdir)
    assert len(glob.glob('{}/*.rpm'.format(pkgdir))) > 0


# each package gets its own subdirectory
def test_download_multiple(utils):
    pkg1 = utils.config["sglversion_pkgname"]
    pkg2 = utils.config["sglversion2_pkgname"]
    utils.erase_package(pkg1)
    utils.erase_package(pkg2)
    os.makedirs(DOWNLOADBASE, exist_ok=True)

    ret = utils.run(['tdnf', 'download', '-y', '--nogpgcheck',
                     '--downloaddir', DOWNLOADBASE, pkg1, pkg2])
    assert ret['retval'] == 0

    pkgdir1 = os.path.join(DOWNLOADBASE, pkg1)
    pkgdir2 = os.path.join(DOWNLOADBASE, pkg2)
    assert os.path.isdir(pkgdir1)
    assert os.path.isdir(pkgdir2)
    assert len(glob.glob('{}/*.rpm'.format(pkgdir1))) > 0
    assert len(glob.glob('{}/*.rpm'.format(pkgdir2))) > 0


# default download: only the standalone package RPM, no dependencies
def test_download_standalone_only(utils):
    pkgname = utils.config["sglversion_pkgname"]
    utils.erase_package(pkgname)
    os.makedirs(DOWNLOADBASE, exist_ok=True)

    ret = utils.run(['tdnf', 'download', '-y', '--nogpgcheck',
                     '--downloaddir', DOWNLOADBASE, pkgname])
    assert ret['retval'] == 0

    pkgdir = os.path.join(DOWNLOADBASE, pkgname)
    rpms = glob.glob('{}/*.rpm'.format(pkgdir))
    # only the package itself, no dep RPMs
    assert len(rpms) == 1
    assert pkgname in os.path.basename(rpms[0])


# --withdeps downloads the package and its full dependency tree
def test_download_withdeps(utils):
    # tdnf-repoquery-requires depends on tdnf-repoquery-base (both in test repo)
    pkgname = 'tdnf-repoquery-requires'
    os.makedirs(DOWNLOADBASE, exist_ok=True)

    ret = utils.run(['tdnf', 'download', '-y', '--nogpgcheck', '--withdeps',
                     '--downloaddir', DOWNLOADBASE, pkgname])
    assert ret['retval'] == 0

    pkgdir = os.path.join(DOWNLOADBASE, pkgname)
    rpms = glob.glob('{}/*.rpm'.format(pkgdir))
    # package RPM plus at least one dependency (tdnf-repoquery-base)
    assert len(rpms) > 1


# download succeeds even when the package is already installed
def test_download_already_installed(utils):
    pkgname = utils.config["sglversion_pkgname"]
    os.makedirs(DOWNLOADBASE, exist_ok=True)

    utils.install_package(pkgname)
    assert utils.check_package(pkgname)

    ret = utils.run(['tdnf', 'download', '-y', '--nogpgcheck',
                     '--downloaddir', DOWNLOADBASE, pkgname])
    assert ret['retval'] == 0
    # package stays installed (download-only)
    assert utils.check_package(pkgname)
    pkgdir = os.path.join(DOWNLOADBASE, pkgname)
    assert os.path.isdir(pkgdir)
    assert len(glob.glob('{}/{}*.rpm'.format(pkgdir, pkgname))) > 0

    utils.erase_package(pkgname)


# if the per-package subdir already exists the operation is aborted
def test_download_dir_already_exists(utils):
    pkgname = utils.config["sglversion_pkgname"]
    utils.erase_package(pkgname)
    os.makedirs(DOWNLOADBASE, exist_ok=True)

    pkgdir = os.path.join(DOWNLOADBASE, pkgname)
    os.makedirs(pkgdir)

    ret = utils.run(['tdnf', 'download', '-y', '--nogpgcheck',
                     '--downloaddir', DOWNLOADBASE, pkgname])
    assert ret['retval'] != 0
    # nothing downloaded into the pre-existing directory
    assert len(glob.glob('{}/*.rpm'.format(pkgdir))) == 0


# success output must contain the path where the package was downloaded
def test_download_shows_path(utils):
    pkgname = utils.config["sglversion_pkgname"]
    utils.erase_package(pkgname)
    os.makedirs(DOWNLOADBASE, exist_ok=True)

    ret = utils.run(['tdnf', 'download', '-y', '--nogpgcheck',
                     '--downloaddir', DOWNLOADBASE, pkgname])
    assert ret['retval'] == 0

    expected_dir = os.path.join(DOWNLOADBASE, pkgname)
    output = '\n'.join(ret.get('stdout', []) + ret.get('stderr', []))
    assert expected_dir in output
