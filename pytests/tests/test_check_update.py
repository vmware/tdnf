#
# Copyright (C) 2019-2023 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import os
import pytest
import shutil

WORKDIR = '/root/test_config'
TEST_CONF_FILE = 'tdnf.conf'
TEST_CONF_PATH = os.path.join(WORKDIR, TEST_CONF_FILE)


@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    tdnf_conf = os.path.join(utils.config['repo_path'], 'tdnf.conf')

    utils.makedirs(WORKDIR)
    shutil.copy(tdnf_conf, TEST_CONF_PATH)

    utils.edit_config({'dnf_check_update_compat': '1'}, section='main', filename=TEST_CONF_PATH)

    yield
    teardown_test(utils)


def teardown_test(utils):
    shutil.rmtree(WORKDIR)
    spkg = utils.config["sglversion_pkgname"]
    mpkg = utils.config["mulversion_pkgname"]
    utils.run(['tdnf', 'erase', '-y', spkg, mpkg])


def test_check_update_no_arg(utils):
    ret = utils.run(['tdnf', 'check-update'])
    assert ret['retval'] == 0


def test_check_update_no_arg_dnf_compat(utils):
    ret = utils.run(['tdnf', '--config', TEST_CONF_PATH, 'check-update'])
    assert ret['retval'] == 0


def test_check_update_invalid_args(utils):
    ret = utils.run(['tdnf', 'check-update', 'abcd', '1234'])
    assert ret['retval'] == 0


def test_check_update_multi_version_package(utils):
    package = utils.config["mulversion_pkgname"] + '-' + utils.config["mulversion_lower"]
    ret = utils.run(['tdnf', 'install', '-y', '--nogpgcheck', package])
    assert utils.check_package(utils.config["mulversion_pkgname"])

    ret = utils.run(['tdnf', 'check-update', utils.config["mulversion_pkgname"]])
    assert len(ret['stdout']) > 0
    assert ret['retval'] == 0


def test_check_update_multi_version_package_dnf_compat(utils):
    package = utils.config["mulversion_pkgname"] + '-' + utils.config["mulversion_lower"]
    ret = utils.run(['tdnf', 'install', '-y', '--nogpgcheck', package])
    assert utils.check_package(utils.config["mulversion_pkgname"])

    ret = utils.run(['tdnf', '--config', TEST_CONF_PATH, 'check-update'])
    assert len(ret['stdout']) > 0
    assert ret['retval'] == 100


def test_check_update_single_version_package(utils):
    package = utils.config["sglversion_pkgname"]
    ret = utils.run(['tdnf', 'install', '-y', '--nogpgcheck', package])
    assert ret['retval'] == 0

    ret = utils.run(['tdnf', 'check-update', package])
    assert len(ret['stdout']) == 0
