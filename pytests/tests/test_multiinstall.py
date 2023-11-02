#
# Copyright (C) 2023 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import pytest


PKGNAME = "tdnf-multi"
# must be sorted:
PKG_VERSIONS = ["1.0.1-1", "1.0.1-2", "1.0.1-3", "1.0.1-4"]


@pytest.fixture(scope='function', autouse=True)
def setup_test(utils):
    utils.edit_config({"installonlypkgs": PKGNAME})
    yield
    teardown_test(utils)


def teardown_test(utils):
    # removing package by name without version will remoe all versions
    pkgname = PKGNAME
    utils.run(['tdnf', 'erase', '-y', pkgname])
    utils.edit_config({"installonlypkgs": None})


# package can be installed twice
def test_install_twice(utils):
    pkgname = PKGNAME
    utils.erase_package(pkgname)

    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkgname])
    # should install latest version
    latest = PKG_VERSIONS[-1]
    assert utils.check_package(pkgname, version=latest)

    first = PKG_VERSIONS[0]
    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', f"{pkgname}={first}"])
    assert utils.check_package(pkgname, version=first)

    # test that the other version is still there
    assert utils.check_package(pkgname, version=latest)


# package can be installed thrice
def test_install_thrice(utils):
    pkgname = PKGNAME
    utils.erase_package(pkgname)

    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkgname])
    # should install latest version
    latest = PKG_VERSIONS[-1]
    assert utils.check_package(pkgname, version=latest)

    first = PKG_VERSIONS[0]
    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', f"{pkgname}={first}"])
    assert utils.check_package(pkgname, version=first)

    second = PKG_VERSIONS[1]
    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', f"{pkgname}={second}"])
    assert utils.check_package(pkgname, version=second)

    # test that the other version is still there
    assert utils.check_package(pkgname, version=latest)
    assert utils.check_package(pkgname, version=first)


# forth install removes the first installed one
def test_install_fourth(utils):
    pkgname = PKGNAME
    utils.erase_package(pkgname)

    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkgname])
    # should install latest version
    latest = PKG_VERSIONS[-1]
    assert utils.check_package(pkgname, version=latest)

    first = PKG_VERSIONS[0]
    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', f"{pkgname}={first}"])
    assert utils.check_package(pkgname, version=first)

    second = PKG_VERSIONS[1]
    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', f"{pkgname}={second}"])
    assert utils.check_package(pkgname, version=second)

    third = PKG_VERSIONS[2]
    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', f"{pkgname}={third}"])
    assert utils.check_package(pkgname, version=third)

    # the first installed should be gone (default installonly_limit=3)
    assert not utils.check_package(pkgname, version=latest)


# remove without version removes all
def test_install_remove_no_version(utils):
    pkgname = PKGNAME
    utils.erase_package(pkgname)

    first = PKG_VERSIONS[0]
    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', f"{pkgname}={first}"])
    assert utils.check_package(pkgname, version=first)

    second = PKG_VERSIONS[1]
    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', f"{pkgname}={second}"])
    assert utils.check_package(pkgname, version=second)

    utils.run(['tdnf', 'remove', '-y', pkgname])
    assert not utils.check_package(pkgname, version=first)
    assert not utils.check_package(pkgname, version=second)


# remove with version removes one only
def test_install_remove_with_version(utils):
    pkgname = PKGNAME
    utils.erase_package(pkgname)

    first = PKG_VERSIONS[0]
    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', f"{pkgname}={first}"])
    assert utils.check_package(pkgname, version=first)

    second = PKG_VERSIONS[1]
    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', f"{pkgname}={second}"])
    assert utils.check_package(pkgname, version=second)

    utils.run(['tdnf', 'remove', '-y', f"{pkgname}={first}"])
    assert not utils.check_package(pkgname, version=first)
    # other pkgs should remain installed:
    assert utils.check_package(pkgname, version=second)


# a reinstall leaves them intact
def test_install_reinstall(utils):
    pkgname = PKGNAME
    utils.erase_package(pkgname)

    first = PKG_VERSIONS[0]
    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', f"{pkgname}={first}"])
    assert utils.check_package(pkgname, version=first)

    second = PKG_VERSIONS[1]
    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', f"{pkgname}={second}"])
    assert utils.check_package(pkgname, version=second)

    utils.run(['tdnf', 'reinstall', '-y', '--nogpgcheck', f"{pkgname}={first}"])
    # both pkgs should remain installed:
    assert utils.check_package(pkgname, version=first)
    assert utils.check_package(pkgname, version=second)
