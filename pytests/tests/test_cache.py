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
    utils.run([ 'cp', '/etc/yum.repos.d/photon-iso.repo', '/etc/yum.repos.d/photon-iso.repo.bak' ])
    utils.run([ 'sed', '-i', 's/enabled=0/enabled=1/g', '/etc/yum.repos.d/photon.repo' ])
    utils.run([ 'sed', '-i', 's/enabled=1/enabled=0/g', '/etc/yum.repos.d/photon-test.repo' ])
    yield
    teardown_test(utils)

def teardown_test(utils):
    utils.run([ 'cp', '/etc/yum.repos.d/photon.repo.bak', '/etc/yum.repos.d/photon.repo' ])
    utils.run([ 'cp', '/etc/yum.repos.d/photon-iso.repo.bak', '/etc/yum.repos.d/photon-iso.repo' ])
    utils.run([ 'sed', '-i', 's/enabled=1/enabled=0/g', '/etc/yum.repos.d/photon.repo' ])
    utils.run([ 'sed', '-i', 's/enabled=0/enabled=1/g', '/etc/yum.repos.d/photon-test.repo' ])

def clean_cache(utils):
    utils.run([ 'rm', '-rf', '/var/cache/tdnf' ])

def enable_cache(utils):
    utils.run([ 'sed', '-i', '/keepcache/d', '/etc/tdnf/tdnf.conf' ])
    utils.run([ 'sed', '-i', '$ a keepcache=true', '/etc/tdnf/tdnf.conf' ])

def disable_cache(utils):
    utils.run([ 'sed', '-i', '/keepcache/d', '/etc/tdnf/tdnf.conf' ])

def check_package_in_cache(utils, pkgname):
    ret = utils.run([ 'find', '/var/cache/tdnf', '-name', pkgname + '*.rpm' ])
    if ret['stdout']:
        return True
    return False

def test_install_without_cache(utils):
    clean_cache(utils)
    disable_cache(utils)

    pkgname = utils.config["sglversion_pkgname"]
    if utils.check_package(pkgname):
        utils.remove_package(pkgname)

    utils.run([ 'tdnf', 'install', '-y', '--nogpgcheck', pkgname ])

    assert (check_package_in_cache(utils, pkgname) == False)

def test_install_with_cache(utils):
    clean_cache(utils)
    enable_cache(utils)

    pkgname = utils.config["sglversion_pkgname"]
    if utils.check_package(pkgname):
        utils.remove_package(pkgname)

    utils.run([ 'tdnf', 'install', '-y', '--nogpgcheck', pkgname ])

    assert (check_package_in_cache(utils, pkgname) == True)

def test_install_with_keepcache_false(utils):
    clean_cache(utils)
    disable_cache(utils)
    utils.run([ 'sed', '-i', '$ a keepcache=false', '/etc/tdnf/tdnf.conf' ])

    pkgname = utils.config["sglversion_pkgname"]
    if utils.check_package(pkgname):
        utils.remove_package(pkgname)

    utils.run([ 'tdnf', 'install', '-y', '--nogpgcheck', pkgname ])

    assert (check_package_in_cache(utils, pkgname) == False)

def test_disable_repo_make_cache(utils):
    before = os.path.getmtime('/var/cache/tdnf/photon/lastrefresh')
    utils.run([ 'tdnf', '--disablerepo=*', 'makecache' ])
    after = os.path.getmtime('/var/cache/tdnf/photon/lastrefresh')
    assert (before == after)

def test_enable_repo_make_cache(utils):
    before = os.path.getmtime('/var/cache/tdnf/photon/lastrefresh')
    utils.run([ 'tdnf', '--disablerepo=*', '--enablerepo=photon', 'makecache' ])
    after = os.path.getmtime('/var/cache/tdnf/photon/lastrefresh')
    assert (before < after)
