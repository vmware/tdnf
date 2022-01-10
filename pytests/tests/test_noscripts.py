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
    pkgname = 'tdnf-bad-pre'
    utils.run(['tdnf', 'erase', '-y', pkgname])


# Verify that the package is really bad:
def test_install_normal(utils):
    pkgname = 'tdnf-bad-pre'
    ret = utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkgname])
    assert(ret['retval'] == 1525)


def test_install_noscripts(utils):
    pkgname = 'tdnf-bad-pre'
    ret = utils.run(['tdnf', 'install', '-y', '--nogpgcheck',
                     '--setopt=tsflags=noscripts', pkgname])
    assert(ret['retval'] == 0)
