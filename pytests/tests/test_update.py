#
# Copyright (C) 2019-2022 VMware, Inc. All Rights Reserved.
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
    mpkg = utils.config["mulversion_pkgname"]
    spkg = utils.config["sglversion_pkgname"]
    utils.run(['tdnf', 'erase', '-y', spkg, mpkg])


def test_update_invalid_arg(utils):
    ret = utils.run(['tdnf', 'update', '-y', 'invalid_package'])
    assert ret['retval'] == 1011


def test_update_single_version_package(utils):
    spkg = utils.config["sglversion_pkgname"]
    if not utils.check_package(spkg):
        utils.install_package(spkg)
    ret = utils.run(['tdnf', 'update', '-y', spkg])
    assert ret['stderr'][0] == 'Nothing to do.'


def test_update_multi_version_package(utils):
    mpkg = utils.config["mulversion_pkgname"]
    mpkg_version = utils.config["mulversion_lower"]

    if utils.check_package(mpkg):
        utils.erase_package(mpkg)

    # install lower version
    utils.install_package(mpkg, mpkg_version)

    # perform an upgrade
    ret = utils.run(['tdnf', 'update', '-y', '--nogpgcheck', mpkg])
    assert ret['retval'] == 0

    # Verify that it cannot be further upgraded
    ret = utils.run(['tdnf', 'update', '-y', mpkg])
    assert ret['stderr'][0] == 'Nothing to do.'
