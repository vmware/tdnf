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
    # remove sglversion_pkgname at the beginning of tests
    pkgname = utils.config['sglversion_pkgname']
    if utils.check_package(pkgname):
        utils.run(['tdnf', 'erase', '-y', pkgname])
    yield
    teardown_test(utils)


def teardown_test(utils):
    pkgname = utils.config['sglversion_pkgname']
    utils.run(['tdnf', 'erase', '-y', pkgname])


def test_assumeno_install(utils):
    pkgname = utils.config['sglversion_pkgname']
    utils.run(['tdnf', '--assumeno', 'install', pkgname])
    assert not utils.check_package(pkgname)


def test_assumeno_erase(utils):
    pkgname = utils.config['sglversion_pkgname']
    utils.run(['tdnf', 'install', '-y', pkgname])
    assert utils.check_package(pkgname)
    utils.run(['tdnf', '--assumeno', 'erase', pkgname])
    assert utils.check_package(pkgname)
