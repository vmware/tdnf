#
# Copyright (C) 2019 - 2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import pytest


@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    pass


def test_erase_no_arg(utils):
    ret = utils.run(['tdnf', 'erase'])
    assert(ret['stderr'][0] == 'Nothing to do.')


def test_erase_invalid_package(utils):
    ret = utils.run(['tdnf', 'erase', 'invalid_package'])
    assert(ret['retval'] == 1011)


def test_erase_package_with_version_suffix(utils):
    pkgname = utils.config["mulversion_pkgname"]
    pkgversion = utils.config["mulversion_lower"]
    utils.install_package(pkgname, pkgversion)

    utils.run(['tdnf', 'erase', '-y', pkgname + '-' + pkgversion])
    assert(not utils.check_package(pkgname))


def test_erase_package_without_version_suffix(utils):
    pkgname = utils.config["mulversion_pkgname"]
    utils.install_package(pkgname)

    utils.run(['tdnf', 'erase', '-y', pkgname])
    assert(not utils.check_package(pkgname))
