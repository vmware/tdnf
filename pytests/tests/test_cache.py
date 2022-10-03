#
# Copyright (C) 2019-2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import os
import fnmatch
import pytest


@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    run_cmd = ['tdnf', 'erase', '-y']
    pkg_list = [
        utils.config["mulversion_pkgname"],
        utils.config["sglversion_pkgname"],
        utils.config["sglversion2_pkgname"],
    ]
    for pkgname in pkg_list:
        utils.erase_package(pkgname)
    utils.run(run_cmd)


def clean_cache(utils):
    utils.run(['rm', '-rf', utils.tdnf_config.get('main', 'cachedir')])


def enable_cache(utils):
    utils.edit_config({'keepcache': 'true'})


def disable_cache(utils):
    utils.edit_config({'keepcache': None})


def switch_cache_path(utils, new_path):
    utils.edit_config({'cachedir': new_path})


def clean_small_cache(utils):
    utils.run(['rm', '-rf', utils.config['small_cache_path']])


def try_mount_small_cache():
    import subprocess
    mount_script = subprocess.Popen(
        './mount-small-cache',
        shell=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        cwd="."
    )
    out, err = mount_script.communicate()
    return mount_script.returncode


def check_package_in_cache(utils, pkgname):
    cache_dir = utils.tdnf_config.get('main', 'cachedir')
    ret = utils.run(['find', cache_dir, '-name', pkgname + '*.rpm'])
    if ret['stdout']:
        return True
    return False


def find_cache_dir(utils, reponame):
    cache_dir = utils.tdnf_config.get('main', 'cachedir')
    for f in os.listdir(cache_dir):
        if fnmatch.fnmatch(f, '{}-*'.format(reponame)):
            return os.path.join(cache_dir, f)
    return None


def test_install_without_cache(utils):
    clean_cache(utils)
    disable_cache(utils)

    pkgname = utils.config["sglversion_pkgname"]
    if utils.check_package(pkgname):
        utils.erase_package(pkgname)

    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkgname])

    assert not check_package_in_cache(utils, pkgname)


def test_install_with_cache(utils):
    clean_cache(utils)
    enable_cache(utils)

    pkgname = utils.config["sglversion_pkgname"]
    if utils.check_package(pkgname):
        utils.erase_package(pkgname)

    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkgname])

    assert check_package_in_cache(utils, pkgname)


def test_install_with_keepcache_false(utils):
    clean_cache(utils)
    disable_cache(utils)

    pkgname = utils.config["sglversion_pkgname"]
    if utils.check_package(pkgname):
        utils.erase_package(pkgname)

    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkgname])

    assert not check_package_in_cache(utils, pkgname)


def test_disable_repo_make_cache(utils):
    cache_dir = find_cache_dir(utils, 'photon-test')
    assert cache_dir is not None
    lastrefresh = os.path.join(cache_dir, 'lastrefresh')
    before = os.path.getmtime(lastrefresh)
    utils.run(['tdnf', '--disablerepo=*', 'makecache'])
    after = os.path.getmtime(lastrefresh)
    assert before == after


def test_enable_repo_make_cache(utils):
    cache_dir = find_cache_dir(utils, 'photon-test')
    assert cache_dir is not None
    lastrefresh = os.path.join(cache_dir, 'lastrefresh')
    before = os.path.getmtime(lastrefresh)
    utils.run(['tdnf', '--disablerepo=*', '--enablerepo=photon-test', 'makecache'])
    after = os.path.getmtime(lastrefresh)
    assert before < after


# -v (verbose) prints progress data
def test_enable_repo_make_cache_verbose(utils):
    cache_dir = find_cache_dir(utils, 'photon-test')
    assert cache_dir is not None
    lastrefresh = os.path.join(cache_dir, 'lastrefresh')
    before = os.path.getmtime(lastrefresh)
    utils.run(['tdnf', '-v', '--disablerepo=*', '--enablerepo=photon-test', 'makecache'])
    after = os.path.getmtime(lastrefresh)
    assert before < after


def test_download_vs_cache_size_single_package(utils):
    clean_cache(utils)
    enable_cache(utils)

    cache_dir = utils.tdnf_config.get('main', 'cachedir')
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)
    ret = utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkgname])

    down_bytes = utils.download_size_to_bytes(ret['stdout'])
    cached_rpm_bytes = sum(utils.get_cached_package_sizes(cache_dir).values())

    assert utils.floats_approx_equal(down_bytes, cached_rpm_bytes)


def test_download_vs_cache_size_multiple_packages(utils):
    clean_cache(utils)
    enable_cache(utils)

    cache_dir = utils.tdnf_config.get('main', 'cachedir')
    run_args = ['tdnf', 'install', '-y', '--nogpgcheck']
    pkg_list = [
        utils.config["mulversion_pkgname"],
        utils.config["sglversion_pkgname"],
        utils.config["sglversion2_pkgname"],
    ]
    for pkgname in pkg_list:
        utils.erase_package(pkgname)
        run_args.append(pkgname)

    ret = utils.run(run_args)
    down_bytes = utils.download_size_to_bytes(ret['stdout'])
    cached_rpm_bytes = sum(utils.get_cached_package_sizes(cache_dir).values())

    assert utils.floats_approx_equal(down_bytes, cached_rpm_bytes)


@pytest.mark.skipif(try_mount_small_cache() != 0, reason="Failed to mount small cache directory.")
def test_cache_directory_out_of_disk_space(utils):
    small_cache_path = utils.config['small_cache_path']
    switch_cache_path(utils, small_cache_path)
    enable_cache(utils)
    clean_small_cache(utils)

    run_args = ['tdnf', 'install', '-y', '--nogpgcheck']
    pkg_list = [utils.config["toolarge_pkgname"]]
    for pkgname in pkg_list:
        utils.erase_package(pkgname)
        run_args.append(pkgname)
    ret = utils.run(run_args)

    switch_cache_path(utils, utils.tdnf_config.get('main', 'cachedir'))
    clean_cache(utils)
    clean_small_cache(utils)
    assert ret['retval'] == 1036
