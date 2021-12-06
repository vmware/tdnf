#
# Copyright (C) 2020 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.

import os
import glob
import pytest
import platform

ARCH=platform.machine()

@pytest.fixture(scope='function', autouse=True)
def setup_test_function(utils):
    pkgname = utils.config["sglversion_pkgname"]
    utils.run(['tdnf', 'erase', '-y', pkgname])
    os.mkdir(os.path.join(utils.config['repo_path'], 'dummydir'))
    yield
    teardown_test(utils)

def teardown_test(utils):
    pkgname = utils.config["sglversion_pkgname"]
    utils.run(['tdnf', 'erase', '-y', pkgname])
    os.rmdir(os.path.join(utils.config['repo_path'], 'dummydir'))

def get_pkg_file_path(utils, pkgname):
    dir = os.path.join(utils.config['repo_path'], 'photon-test', 'RPMS', ARCH)
    matches = glob.glob('{}/{}-*.rpm'.format(dir, pkgname))
    return matches[0]

def get_pkg_file_path_with_doubledots(utils, pkgname):
    dir = os.path.join(utils.config['repo_path'], 'dummydir', '..', 'photon-test', 'RPMS', ARCH)
    matches = glob.glob('{}/{}-*.rpm'.format(dir, pkgname))
    return matches[0]

def get_pkg_remote_url(utils, pkgname):
    path = get_pkg_file_path(utils, pkgname)
    url = "http://localhost:8080/{}".format(path[len(utils.config['repo_path'])+1:])
    return url

def get_pkg_remote_url_with_doubledots(utils, pkgname):
    path = get_pkg_file_path(utils, pkgname)
    url = "http://localhost:8080/dummydir/../{}".format(path[len(utils.config['repo_path'])+1:])
    return url

# test something like "tdnf install /path/to/pkg.rpm"
def test_install_as_file(utils):
    pkgname = utils.config["sglversion_pkgname"]
    path = get_pkg_file_path(utils, pkgname)
    ret = utils.run([ 'tdnf', 'install', '-y', '--nogpgcheck', path])
    assert(ret['retval']  == 0)
    assert(utils.check_package(pkgname) == True)

# test something like "tdnf install ../path/to/pkg.rpm" (relative path)
def test_install_as_file_relpath(utils):
    pkgname = utils.config["sglversion_pkgname"]
    path = get_pkg_file_path(utils, pkgname)
    filename = os.path.basename(path) 
    os.mkdir('rpmtmp')
    shutil.copy(path, 'rpmtmp')
    relpath = os.path.join('..', 'rpmtmp', filename)
    ret = utils.run([ 'tdnf', 'install', '-y', '--nogpgcheck', relpath])
    assert(ret['retval']  == 0)
    assert(utils.check_package(pkgname) == True)
    shutil.rmtree('rpmtmp')

# test something like "tdnf install /somepath/../path/to/pkg.rpm"
def test_install_as_file_with_doubledots(utils):
    pkgname = utils.config["sglversion_pkgname"]
    path = get_pkg_file_path_with_doubledots(utils, pkgname)
    ret = utils.run([ 'tdnf', 'install', '-y', '--nogpgcheck', path])
    assert(ret['retval']  == 0)
    assert(utils.check_package(pkgname) == True)

# test something like "tdnf install pkg.rpm"
def test_install_as_file_relpath(utils):
    pkgname = utils.config["sglversion_pkgname"]
    path = os.path.relpath(get_pkg_file_path(utils, pkgname))
    ret = utils.run([ 'tdnf', 'install', '-y', '--nogpgcheck', path])
    assert(ret['retval']  == 0)
    assert(utils.check_package(pkgname) == True)

# test something like "tdnf install file:///path/to/pkg.rpm"
def test_install_as_file_uri(utils):
    pkgname = utils.config["sglversion_pkgname"]
    path = get_pkg_file_path(utils, pkgname)
    uri = 'file://{}'.format(path)
    ret = utils.run([ 'tdnf', 'install', '-y', '--nogpgcheck', uri])
    assert(ret['retval']  == 0)
    assert(utils.check_package(pkgname) == True)

# test something like "tdnf install http://server.com/path/to/pkg.rpm"
def test_install_remote(utils):
    pkgname = utils.config["sglversion_pkgname"]
    uri = get_pkg_remote_url(utils, pkgname)
    ret = utils.run([ 'tdnf', 'install', '-y', '--nogpgcheck', uri])
    assert(ret['retval']  == 0)
    assert(utils.check_package(pkgname) == True)

# test something like "tdnf install http://server.com/otherpath/../path/to/pkg.rpm"
def test_install_remote_with_doubledots(utils):
    pkgname = utils.config["sglversion_pkgname"]
    uri = get_pkg_remote_url_with_doubledots(utils, pkgname)
    ret = utils.run([ 'tdnf', 'install', '-y', '--nogpgcheck', uri])
    assert(ret['retval']  == 0)
    assert(utils.check_package(pkgname) == True)

# test something like "tdnf install http://server.com/path/to/pkg.rpm",
# but file doesn't exist, expect failure
def test_install_remote_notfound(utils):
    pkgname = utils.config["sglversion_pkgname"]
    uri = 'http://localhost:8080/doesnotexist.rpm'
    ret = utils.run([ 'tdnf', 'install', '-y', '--nogpgcheck', uri])
    assert(ret['retval']  == 1622)

# test something like "tdnf install /path/to/pkg.rpm otherpkg"
def test_install_as_mixed(utils):
    pkgname = utils.config["sglversion_pkgname"]
    pkgname2 = utils.config["sglversion2_pkgname"]
    path = get_pkg_file_path(utils, pkgname)
    ret = utils.run([ 'tdnf', 'install', '-y', '--nogpgcheck', path, pkgname2])
    assert(ret['retval']  == 0)
    assert(utils.check_package(pkgname) == True)
    assert(utils.check_package(pkgname2) == True)

# test installing a package that has the same name as a file
# example: touch foo; tdnf install foo
# (file needs to have "*.rpm" extension to qualify)
def test_install_same_as_filname(utils):
    pkgname = utils.config["sglversion_pkgname"]
    utils.run(['touch', pkgname])
    ret = utils.run([ 'tdnf', 'install', '-y', '--nogpgcheck', pkgname])
    assert(ret['retval']  == 0)
    assert(utils.check_package(pkgname) == True)
