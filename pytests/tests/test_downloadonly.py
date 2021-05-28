#
# Copyright (C) 2019 - 2020 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.

import os
import shutil
import glob
import pytest

DOWNLOADDIR='/tmp/tdnf/download'

def setup_test_function(utils):
    yield
    teardown_test(utils)

def teardown_test(utils):
    shutil.rmtree(DOWNLOADDIR)

def check_package_in_cache(utils, pkgname):
    cache_dir = utils.tdnf_config.get('main', 'cachedir')
    ret = utils.run([ 'find', cache_dir, '-name', pkgname + '*.rpm' ])
    if ret['stdout']:
        return True
    return False

# download only to cache dir - must succeed but not install, file must
# be in cache dir
def test_install_download_only(utils):
    pkgname = utils.config["sglversion_pkgname"]
    ret = utils.run([ 'tdnf', 'install', '-y', '--downloadonly', pkgname])
    assert(ret['retval']  == 0)
    assert(not utils.check_package(pkgname))
    assert(check_package_in_cache(utils, pkgname))

# download only to cache dir - must succeed but not install, file must
# be in specified directory
def test_install_download_only_to_directory(utils):
    pkgname = utils.config["sglversion_pkgname"]
    os.makedirs(DOWNLOADDIR, exist_ok=True)
    ret = utils.run([ 'tdnf', 'install', '-y', '--downloadonly', '--downloaddir', DOWNLOADDIR, pkgname])
    print (ret)
    assert(ret['retval']  == 0)
    assert(not utils.check_package(pkgname))
    assert(len(glob.glob('{}/{}*.rpm'.format(DOWNLOADDIR, pkgname))) > 0)

# --downloaddir option without --downloadonly should fail
def test_install_downloaddir_no_downloadonly(utils):
    pkgname = utils.config["sglversion_pkgname"]
    ret = utils.run([ 'tdnf', 'install', '-y', '--downloaddir', DOWNLOADDIR, pkgname])
    assert(ret['retval']  != 0)
    assert(not utils.check_package(pkgname))

