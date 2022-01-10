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
    yield
    teardown_test(utils)


def teardown_test(utils):
    pkgname = utils.config["sglversion_pkgname"]
    utils.run(['tdnf', 'erase', '-y', pkgname])


def clean_cache(utils):
    utils.run(['rm', '-rf', utils.tdnf_config.get('main', 'cachedir')])


def enable_cache(utils):
    tdnf_config = os.path.join(utils.config['repo_path'], 'tdnf.conf')
    utils.run(['sed', '-i', '/keepcache/d', tdnf_config])
    utils.run(['sed', '-i', '$ a keepcache=true', tdnf_config])


def disable_cache(utils):
    tdnf_config = os.path.join(utils.config['repo_path'], 'tdnf.conf')
    utils.run(['sed', '-i', '/keepcache/d', tdnf_config])


def check_package_in_cache(utils, pkgname):
    cache_dir = utils.tdnf_config.get('main', 'cachedir')
    ret = utils.run(['find', cache_dir, '-name', pkgname + '*.rpm'])
    if ret['stdout']:
        return True
    return False


def test_install_without_cache(utils):
    clean_cache(utils)
    disable_cache(utils)

    pkgname = utils.config["sglversion_pkgname"]
    if utils.check_package(pkgname):
        utils.erase_package(pkgname)

    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkgname])

    assert(not check_package_in_cache(utils, pkgname))


def test_install_with_cache(utils):
    clean_cache(utils)
    enable_cache(utils)

    pkgname = utils.config["sglversion_pkgname"]
    if utils.check_package(pkgname):
        utils.erase_package(pkgname)

    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkgname])

    assert(check_package_in_cache(utils, pkgname))


def test_install_with_keepcache_false(utils):
    clean_cache(utils)
    disable_cache(utils)
    tdnf_config = os.path.join(utils.config['repo_path'], 'tdnf.conf')
    utils.run(['sed', '-i', '$ a keepcache=false', tdnf_config])

    pkgname = utils.config["sglversion_pkgname"]
    if utils.check_package(pkgname):
        utils.erase_package(pkgname)

    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkgname])

    assert(not check_package_in_cache(utils, pkgname))


def test_disable_repo_make_cache(utils):
    cache_dir = utils.tdnf_config.get('main', 'cachedir')
    lastrefresh = os.path.join(cache_dir, 'photon-test/lastrefresh')
    before = os.path.getmtime(lastrefresh)
    utils.run(['tdnf', '--disablerepo=*', 'makecache'])
    after = os.path.getmtime(lastrefresh)
    assert(before == after)


def test_enable_repo_make_cache(utils):
    cache_dir = utils.tdnf_config.get('main', 'cachedir')
    lastrefresh = os.path.join(cache_dir, 'photon-test/lastrefresh')
    before = os.path.getmtime(lastrefresh)
    utils.run(['tdnf', '--disablerepo=*', '--enablerepo=photon-test', 'makecache'])
    after = os.path.getmtime(lastrefresh)
    assert(before < after)


# -v (verbose) prints progress data
def test_enable_repo_make_cache_verbose(utils):
    cache_dir = utils.tdnf_config.get('main', 'cachedir')
    lastrefresh = os.path.join(cache_dir, 'photon-test/lastrefresh')
    before = os.path.getmtime(lastrefresh)
    utils.run(['tdnf', '-v', '--disablerepo=*', '--enablerepo=photon-test', 'makecache'])
    after = os.path.getmtime(lastrefresh)
    assert(before < after)
