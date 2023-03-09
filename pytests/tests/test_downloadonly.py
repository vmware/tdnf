#
# Copyright (C) 2019 - 2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.

import os
import shutil
import glob
import pytest

DOWNLOADDIR = '/tmp/tdnf/download'


@pytest.fixture(scope='function', autouse=True)
def setup_test_function(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    if os.path.isdir(DOWNLOADDIR):
        shutil.rmtree(DOWNLOADDIR)


def check_package_in_cache(utils, pkgname):
    cache_dir = utils.tdnf_config.get('main', 'cachedir')
    ret = utils.run(['find', cache_dir, '-name', pkgname + '*.rpm'])
    if ret['stdout']:
        return True
    return False


# download only to cache dir - must succeed but not install, file must
# be in cache dir
def test_install_download_only(utils):
    pkgname = utils.config["sglversion_pkgname"]
    ret = utils.run(['tdnf', 'install', '-y', '--downloadonly', pkgname])
    assert ret['retval'] == 0
    assert not utils.check_package(pkgname)
    assert check_package_in_cache(utils, pkgname)


# download only to cache dir - must succeed but not install, file must
# be in specified directory
def test_install_download_only_to_directory(utils):
    pkgname = utils.config["sglversion_pkgname"]
    os.makedirs(DOWNLOADDIR, exist_ok=True)
    ret = utils.run(['tdnf', 'install', '-y', '--downloadonly', '--downloaddir', DOWNLOADDIR, pkgname])
    print(ret)
    assert ret['retval'] == 0
    assert not utils.check_package(pkgname)
    assert len(glob.glob('{}/{}*.rpm'.format(DOWNLOADDIR, pkgname))) > 0


# --downloaddir option without --downloadonly should fail
def test_install_downloaddir_no_downloadonly(utils):
    pkgname = utils.config["sglversion_pkgname"]
    ret = utils.run(['tdnf', 'install', '-y', '--downloaddir', DOWNLOADDIR, pkgname])
    assert ret['retval'] != 0
    assert not utils.check_package(pkgname)


# --alldeps option without --downloadonly should fail
def test_install_aldeps_no_downloadonly(utils):
    pkgname = utils.config["sglversion_pkgname"]
    ret = utils.run(['tdnf', 'install', '-y', '--alldeps', DOWNLOADDIR, pkgname])
    assert ret['retval'] != 0
    assert not utils.check_package(pkgname)


# --nodeps option without --downloadonly should fail
def test_install_nodeps_no_downloadonly(utils):
    pkgname = utils.config["sglversion_pkgname"]
    ret = utils.run(['tdnf', 'install', '-y', '--nodeps', DOWNLOADDIR, pkgname])
    assert ret['retval'] != 0
    assert not utils.check_package(pkgname)


# an uninstalled requirement will be downloaded as well (unless --nodeps is set):
def test_install_download_only_and_requires(utils):
    pkgname = 'tdnf-test-cleanreq-leaf1'
    pkgname_req = 'tdnf-test-cleanreq-required'

    utils.erase_package(pkgname_req)
    assert not utils.check_package(pkgname_req)

    os.makedirs(DOWNLOADDIR, exist_ok=True)

    ret = utils.run(['tdnf', 'install', '-y', '--downloadonly', '--downloaddir', DOWNLOADDIR, pkgname])

    assert ret['retval'] == 0
    assert not utils.check_package(pkgname)
    assert len(glob.glob('{}/{}*.rpm'.format(DOWNLOADDIR, pkgname_req))) > 0


# normally an installed requirement will not be downloaded, but with --alldeps it should:
def test_install_download_only_alldeps(utils):
    pkgname = 'tdnf-test-cleanreq-leaf1'
    pkgname_req = 'tdnf-test-cleanreq-required'

    utils.install_package(pkgname_req)
    assert utils.check_package(pkgname_req)

    os.makedirs(DOWNLOADDIR, exist_ok=True)

    ret = utils.run(['tdnf', 'install', '-y', '--downloadonly', '--downloaddir', DOWNLOADDIR, '--alldeps', pkgname])

    assert ret['retval'] == 0
    assert not utils.check_package(pkgname)
    assert len(glob.glob('{}/{}*.rpm'.format(DOWNLOADDIR, pkgname_req))) > 0


# normally an uninstalled requirement will be downloaded, but with --nodeps it should not:
def test_install_download_only_nodeps(utils):
    pkgname = 'tdnf-test-cleanreq-leaf1'
    pkgname_req = 'tdnf-test-cleanreq-required'

    utils.erase_package(pkgname_req)
    assert not utils.check_package(pkgname_req)

    os.makedirs(DOWNLOADDIR, exist_ok=True)

    ret = utils.run(['tdnf', 'install', '-y', '--downloadonly', '--downloaddir', DOWNLOADDIR, '--nodeps', pkgname])

    assert ret['retval'] == 0
    assert not utils.check_package(pkgname)
    assert len(glob.glob('{}/{}*.rpm'.format(DOWNLOADDIR, pkgname_req))) == 0
