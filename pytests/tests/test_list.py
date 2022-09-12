#
# Copyright (C) 2019-2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import pytest


@pytest.fixture(scope='function', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    mpkg = utils.config["mulversion_pkgname"]
    spkg = utils.config["sglversion_pkgname"]
    utils.run(['tdnf', 'erase', '-y', mpkg, spkg])


def helper_test_list_sub_cmd(utils, sub_cmd):
    ret = utils.run(['tdnf', 'list', sub_cmd])
    assert ret['retval'] == 0
    ret = utils.run(['tdnf', 'list', sub_cmd, utils.config["sglversion_pkgname"]])
    assert ret['retval'] == 0
    ret = utils.run(['tdnf', 'list', sub_cmd, 'invalid_package'])
    assert ret['retval'] == 1011


def test_list_top(utils):
    spkg = utils.config["sglversion_pkgname"]
    ret = utils.run(['tdnf', 'install', '-y', spkg])
    assert ret['retval'] == 0

    ret = utils.run(['tdnf', 'list'])
    assert ret['retval'] == 0
    ret = utils.run(['tdnf', 'list', utils.config["sglversion_pkgname"]])
    assert ret['retval'] == 0
    ret = utils.run(['tdnf', 'list', 'invalid_package'])
    assert ret['retval'] == 1011


def test_list_sub_cmd(utils):
    spkg = utils.config["sglversion_pkgname"]
    ret = utils.run(['tdnf', 'install', '-y', spkg])
    assert ret['retval'] == 0

    helper_test_list_sub_cmd(utils, 'all')
    helper_test_list_sub_cmd(utils, 'installed')
    helper_test_list_sub_cmd(utils, 'available')
    helper_test_list_sub_cmd(utils, 'obsoletes')
    helper_test_list_sub_cmd(utils, 'extras')
    helper_test_list_sub_cmd(utils, 'recent')


def test_list_options(utils):
    spkg = utils.config["sglversion_pkgname"]
    ret = utils.run(['tdnf', 'install', '-y', spkg])
    assert ret['retval'] == 0

    helper_test_list_sub_cmd(utils, '--all')
    helper_test_list_sub_cmd(utils, '--installed')
    helper_test_list_sub_cmd(utils, '--available')
    helper_test_list_sub_cmd(utils, '--obsoletes')
    helper_test_list_sub_cmd(utils, '--extras')
    helper_test_list_sub_cmd(utils, '--recent')


def test_list_notinstalled(utils):
    mpkg = utils.config["mulversion_pkgname"]
    mpkg_version = utils.config["mulversion_lower"]
    spkg = utils.config["sglversion_pkgname"]

    ret = utils.run(['tdnf', 'list', 'upgrades'])
    assert ret['retval'] == 0

    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', mpkg + '-' + mpkg_version])
    ret = utils.run(['tdnf', 'list', 'upgrades', mpkg])
    assert len(ret['stdout']) == 1

    for cmd in ['updates', 'upgrades', 'downgrades', '--updates', '--upgrades', '--downgrades']:
        ret = utils.run(['tdnf', 'list', cmd, 'invalid_package'])
        assert ret['retval'] == 1011

        # spkg is not installed, expect error
        ret = utils.run(['tdnf', 'list', cmd, spkg])
        assert ret['retval'] == 1011
