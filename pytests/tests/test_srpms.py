#
# Copyright (C) 2019 - 2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import glob
import os
import platform
import pytest
import shutil

ARCH = platform.machine()

DIST = os.environ.get('DIST')
# dir that holds the SPECS dir:
if DIST == 'fedora':
    RPMBUILD_DIR = '/root/rpmbuild'
else:
    RPMBUILD_DIR = '/usr/src/photon'


@pytest.fixture(scope='function', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    if (os.path.isdir(RPMBUILD_DIR)):
        shutil.rmtree(RPMBUILD_DIR)


def get_pkg_file_path(utils, pkgname):
    dir = os.path.join(utils.config['repo_path'], 'photon-test', 'RPMS', ARCH)
    matches = glob.glob('{}/{}-*.rpm'.format(dir, pkgname))
    return matches[0]


def get_srcpkg_file_path(utils, pkgname):
    dir = os.path.join(utils.config['repo_path'], 'photon-test-src', 'SRPMS')
    matches = glob.glob('{}/{}-*.src.rpm'.format(dir, pkgname))
    return matches[0]


def test_install_srpm(utils):
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)

    ret = utils.run(['tdnf', 'install', '--repoid=photon-test-src', '-y', '--source', '--nogpgcheck', pkgname])
    assert ret['retval'] == 0
    assert not utils.check_package(pkgname)  # source RPMs are never really installed

    assert len(glob.glob(os.path.join(RPMBUILD_DIR, 'SPECS', '*.spec'))) > 0


def test_install_srpm_file_with_source_option(utils):
    pkgname = utils.config["sglversion_pkgname"]
    path = get_srcpkg_file_path(utils, pkgname)
    ret = utils.run(['tdnf', 'install', '-y', '--source', '--nogpgcheck', path])
    assert ret['retval'] == 0

    assert len(glob.glob(os.path.join(RPMBUILD_DIR, 'SPECS', '*.spec'))) > 0


# fail if trying to install an rpm with --source option
def test_install_rpm_file_with_source_option(utils):
    pkgname = utils.config["sglversion_pkgname"]
    path = get_pkg_file_path(utils, pkgname)
    ret = utils.run(['tdnf', 'install', '-y', '--source', '--nogpgcheck', path])
    assert ret['retval'] != 0


# fail if trying to install an srpm without --source option
def test_install_srpm_file_without_source_option(utils):
    pkgname = utils.config["sglversion_pkgname"]
    path = get_srcpkg_file_path(utils, pkgname)
    ret = utils.run(['tdnf', 'install', '-y', '--nogpgcheck', path])
    assert ret['retval'] != 0
