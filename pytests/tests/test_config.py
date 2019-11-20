#
# Copyright (C) 2019 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#
#   Author: Siddharth Chandrasekaran <csiddharth@vmware.com>

import os
import pytest

@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    utils.run([ 'cp', '/etc/yum.repos.d/photon.repo', '/etc/yum.repos.d/photon.repo.bak' ])
    utils.run([ 'sed', '-i', 's/enabled=1/enabled=0/g', '/etc/yum.repos.d/photon.repo' ])
    utils.run([ 'cp', '/etc/tdnf/tdnf.conf', '/etc/tdnf/mytdnf.conf' ])
    utils.run([ 'sed', '-i', '/repodir/d', '/etc/tdnf/mytdnf.conf' ])
    utils.run([ 'sed', '-i', '$ a repodir=/etc/myrepo', '/etc/tdnf/mytdnf.conf' ])
    utils.run([ 'mkdir', '-p', '/etc/myrepo' ])
    utils.run([ 'cp', '/etc/yum.repos.d/photon.repo', '/etc/myrepo/photon.repo' ])
    utils.run([ 'sed', '-i', 's/enabled=0/enabled=1/g', '/etc/myrepo/photon.repo' ])
    yield
    teardown_test(utils)

def teardown_test(utils):
    utils.run([ 'cp', '/etc/yum.repos.d/photon.repo.bak', '/etc/yum.repos.d/photon.repo' ])
    utils.run([ 'rm', '/etc/tdnf/mytdnf.conf' ])
    utils.run([ 'rm', '-rf', '/etc/myrepo' ])

def test_config_invalid(utils):
    ret = utils.run([ 'tdnf', '--config', '/etc/tdnf/test123.conf', 'list', 'tdnf' ])
    assert(ret['retval'] == 66)

def test_config_list(utils):
    ret = utils.run([ 'tdnf', '--config', '/etc/tdnf/mytdnf.conf', 'list', 'tdnf' ])
    assert(ret['retval'] == 0)

def test_config_list_with_disable_repos(utils):
    ret = utils.run([ 'tdnf', '--disablerepo=*', '--config', '/etc/tdnf/mytdnf.conf', 'list', 'tdnf' ])
    assert(ret['retval'] == 0)

    for line in ret['stdout']:
        if not line or '@System' in line:
            continue
        assert (False) # force fail test

def test_config_invaid_repodir(utils):
    utils.run([ 'sed', '-i', 's#repodir=/etc/myrepo#repodir=/etc/invalid#g', '/etc/tdnf/mytdnf.conf' ])
    ret = utils.run([ 'tdnf', '--config', '/etc/tdnf/mytdnf.conf', 'list', 'tdnf' ])
    assert(ret['retval'] == 45)
