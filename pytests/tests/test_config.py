#
# Copyright (C) 2019-2022 VMware, Inc. All Rights Reserved.
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
TEST_REPO = 'photon-test'
TEST_REPO_FILE = TEST_REPO + '.repo'
TEST_CONF_PATH = os.path.join(WORKDIR, TEST_CONF_FILE)
TEST_REPO_PATH = os.path.join(WORKDIR, TEST_REPO_FILE)


@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    tdnf_conf = os.path.join(utils.config['repo_path'], 'tdnf.conf')
    tdnf_repo = os.path.join(utils.tdnf_config.get('main', 'repodir'), 'photon-test.repo')

    utils.makedirs(WORKDIR)
    shutil.copy(tdnf_conf, TEST_CONF_PATH)
    shutil.copy(tdnf_repo, TEST_REPO_PATH)

    utils.edit_config({'repodir': WORKDIR}, section='main', filename=TEST_CONF_PATH)

    utils.edit_config({'enabled': '1'}, section=TEST_REPO, filename=TEST_REPO_PATH)

    yield
    teardown_test(utils)


def teardown_test(utils):
    shutil.rmtree(WORKDIR)
    pkg = utils.config['sglversion_pkgname']
    utils.run(['tdnf', 'erase', '-y', pkg])


def test_config_invalid(utils):
    spkg = utils.config['sglversion_pkgname']
    ret = utils.run(['tdnf', '--config', os.path.join(WORKDIR, 'test123.conf'), 'list', spkg])
    assert ret['retval'] == 1602


def test_config_list(utils):
    spkg = utils.config['sglversion_pkgname']
    ret = utils.run(['tdnf', 'install', '-y', spkg])
    assert ret['retval'] == 0
    ret = utils.run(['tdnf', '--config', TEST_CONF_PATH, 'list', spkg])
    assert ret['retval'] == 0


def test_config_list_with_disable_repos(utils):
    spkg = utils.config['sglversion_pkgname']
    ret = utils.run(['tdnf', '--disablerepo=*', '--config', TEST_CONF_PATH, 'list', spkg])
    assert ret['retval'] == 0

    for line in ret['stdout']:
        if not line or '@System' in line:
            continue
        assert (False)  # force fail test


def test_config_invaid_repodir(utils):
    pkg = utils.config['sglversion_pkgname']
    utils.edit_config({'repodir': '/etc/invalid'}, section='main', filename=TEST_CONF_PATH)
    ret = utils.run(['tdnf', '--config', TEST_CONF_PATH, 'list', pkg])
    assert ret['retval'] == 1005
