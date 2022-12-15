#
# Copyright (C) 2019 - 2020 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#
#   Author: Siddharth Chandrasekaran <csiddharth@vmware.com>

import os
import tempfile
import pytest

PKGNAME_OBSED_VER = "tdnf-test-dummy-obsoleted=0.1"
PKGNAME_OBSED = "tdnf-test-dummy-obsoleted"
PKGNAME_OBSING = "tdnf-test-dummy-obsoleting"


@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    pkgname = utils.config["mulversion_pkgname"]
    utils.run(['tdnf', 'erase', '-y', pkgname])


def test_install_no_arg(utils):
    ret = utils.run([ 'tdnf', 'install' ])
    assert(ret['retval'] == 1001)


def test_install_invalid_arg(utils):
    ret = utils.run([ 'tdnf', 'install', 'invalid_package' ])
    assert(ret['retval'] == 1011)


def test_install_package_with_version_suffix(utils):
    pkgname = utils.config["mulversion_pkgname"]
    pkgversion = utils.config["mulversion_lower"]
    utils.erase_package(pkgname)

    utils.run([ 'tdnf', 'install', '-y', '--nogpgcheck', pkgname + '-' + pkgversion ])
    assert(utils.check_package(pkgname) == True)


def test_install_package_without_version_suffix(utils):
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)

    utils.run([ 'tdnf', 'install', '-y', '--nogpgcheck', pkgname ])
    assert(utils.check_package(pkgname) == True)

# -v (verbose) prints progress data
def test_install_package_verbose(utils):
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)

    utils.run([ 'tdnf', 'install', '-y','-v', '--nogpgcheck', pkgname ])
    assert(utils.check_package(pkgname) == True)


def test_dummy_requires(utils):
    pkg = utils.config["dummy_requires_pkgname"]
    ret = utils.run(['tdnf', 'install', '-y', pkg])
    assert ' nothing provides ' in ret['stderr'][0]


def test_install_testonly(utils):
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)

    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', '--testonly', pkgname])
    assert not utils.check_package(pkgname)


# install an obsoleted package, expect the obsoleting package to be installed
# the obsoleting package must also provide the obsoleted one
def test_install_obsoletes(utils):
    utils.erase_package(PKGNAME_OBSED)
    utils.erase_package(PKGNAME_OBSING)

    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', PKGNAME_OBSED])
    assert utils.check_package(PKGNAME_OBSING)


# install an obsoleted package with version - expect the obsoleted package to be installed
def test_install_obsoleted_version(utils):
    utils.erase_package(PKGNAME_OBSED_VER)
    utils.erase_package(PKGNAME_OBSING)

    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', PKGNAME_OBSED_VER])
    assert utils.check_package(PKGNAME_OBSED)


# same as test_install_obsoletes, but the obsoleted package already installed
def test_install_obsoleted_installed(utils):
    # make sure we install the obsoleted one by using version
    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', PKGNAME_OBSED_VER])
    utils.erase_package(PKGNAME_OBSING)

    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', PKGNAME_OBSED])
    assert utils.check_package(PKGNAME_OBSING)


def test_install_memcheck(utils):
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)

    utils.run_memcheck(['tdnf', 'install', '-y', '--nogpgcheck', pkgname])
    assert utils.check_package(pkgname)
