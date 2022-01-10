#
# Copyright (C) 2019-2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import os
import pytest


@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    tdnf_conf = os.path.join(utils.config['repo_path'], 'tdnf.conf')
    tdnf_repo = os.path.join(utils.tdnf_config.get('main', 'repodir'), 'photon-test.repo')
    utils.run(['mkdir', '-p', '/tmp/myrepo'])
    utils.run(['cp', tdnf_conf, '/tmp/myrepo/mytdnf.conf'])
    utils.run(['sed', '-i', '/repodir/d', '/tmp/myrepo/mytdnf.conf'])
    utils.run(['sed', '-i', '$ a repodir=/tmp/myrepo', '/tmp/myrepo/mytdnf.conf'])
    utils.run(['cp', tdnf_repo, '/tmp/myrepo/photon.repo'])
    utils.run(['sed', '-i', 's/enabled=0/enabled=1/g', '/tmp/myrepo/photon.repo'])
    yield
    teardown_test(utils)


def teardown_test(utils):
    spkg = utils.config['sglversion_pkgname']
    utils.run(['rm', '-rf', '/tmp/myrepo'])
    utils.run(['tdnf', 'erase', '-y', spkg])


def test_config_invalid(utils):
    spkg = utils.config['sglversion_pkgname']
    ret = utils.run(['tdnf', '--config', '/tmp/myrepo/test123.conf', 'list', spkg])
    assert(ret['retval'] == 1602)


def test_config_list(utils):
    spkg = utils.config['sglversion_pkgname']
    ret = utils.run(['tdnf', 'install', '-y', spkg])
    assert(ret['retval'] == 0)
    ret = utils.run(['tdnf', '--config', '/tmp/myrepo/mytdnf.conf', 'list', spkg])
    assert(ret['retval'] == 0)


def test_config_list_with_disable_repos(utils):
    spkg = utils.config['sglversion_pkgname']
    ret = utils.run(['tdnf', '--disablerepo=*', '--config', '/tmp/myrepo/mytdnf.conf', 'list', spkg])
    assert(ret['retval'] == 0)

    for line in ret['stdout']:
        if not line or '@System' in line:
            continue
        assert (False)  # force fail test


def test_config_invaid_repodir(utils):
    spkg = utils.config['sglversion_pkgname']
    utils.run(['sed', '-i', 's#repodir=/tmp/myrepo#repodir=/etc/invalid#g', '/tmp/myrepo/mytdnf.conf'])
    ret = utils.run(['tdnf', '--config', '/tmp/myrepo/mytdnf.conf', 'list', spkg])
    assert(ret['retval'] == 1005)
