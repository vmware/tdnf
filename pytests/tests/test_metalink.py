#
# Copyright (C) 2020 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#
#   Author: Tapas Kundu <tkundu@vmware.com>

import os
import pytest

metalink_file_path = 'photon-test/metalink'
repomd_file_path = 'photon-test/repodata/repomd.xml'

@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)

#while exiting, uncomment baseurl and comment metalink
def teardown_test(utils):
    set_baseurl(utils, True)
    set_metalink(utils, False)
    set_md5(utils, False)
    set_sha1(utils, False)
    set_sha256(utils, False)

def set_baseurl(utils, enabled):
    tdnf_repo = os.path.join(utils.tdnf_config.get('main', 'repodir'), 'photon-test.repo')
    if enabled:
        utils.run([ 'sed', '-i', '/baseurl/s/^#//g', tdnf_repo ])
    else:
        utils.run([ 'sed', '-e', '/baseurl/ s/^#*/#/g', '-i', tdnf_repo ])

def set_metalink(utils, enabled):
    tdnf_repo = os.path.join(utils.tdnf_config.get('main', 'repodir'), 'photon-test.repo')
    if enabled:
        utils.run([ 'sed', '-i', '/metalink/s/^#//g', tdnf_repo ])
    else:
        utils.run([ 'sed', '-e', '/metalink/ s/^#*/#/g', '-i', tdnf_repo ])

def set_md5(utils, enabled):
    photon_metalink = os.path.join(utils.config['repo_path'], metalink_file_path)
    if enabled:
        repomd_file = os.path.join(utils.config['repo_path'], repomd_file_path)
        ret = utils.run([ 'md5sum', repomd_file])
        md5sum = ret['stdout'][0].split()[0]
        utils.run([ 'sed', '-i', '-e' ,'/<verification>/a \    <hash type="md5">' + md5sum + '</hash>', photon_metalink ])
    else:
        utils.run([ 'sed', '-i', '-e', '/type="md5"/d', photon_metalink])


def set_sha1(utils, enabled):
    photon_metalink = os.path.join(utils.config['repo_path'], metalink_file_path)
    if enabled:
        repomd_file = os.path.join(utils.config['repo_path'], repomd_file_path)
        ret = utils.run([ 'sha1sum', repomd_file])
        sha1sum = ret['stdout'][0].split()[0]
        utils.run([ 'sed', '-i', '-e' ,'/<verification>/a \    <hash type="sha1">' + sha1sum + '</hash>', photon_metalink ])
    else:
        utils.run([ 'sed', '-i', '-e', '/type="sha1"/d', photon_metalink])

def set_sha256(utils, enabled):
    photon_metalink = os.path.join(utils.config['repo_path'], metalink_file_path)
    if enabled:
        repomd_file = os.path.join(utils.config['repo_path'], repomd_file_path)
        ret = utils.run([ 'sha256sum', repomd_file])
        sha256sum = ret['stdout'][0].split()[0]
        utils.run([ 'sed', '-i', '-e' ,'/<verification>/a \    <hash type="sha256">' + sha256sum + '</hash>', photon_metalink ])
    else:
        utils.run([ 'sed', '-i', '-e', '/type="sha256"/d', photon_metalink])

def set_invalid_md5(utils):
    photon_metalink = os.path.join(utils.config['repo_path'], metalink_file_path)
    md5sum = 'f894a23a50e757b8aae25596ceb04777'
    utils.run([ 'sed', '-i', '-e' ,'/<verification>/a \    <hash type="md5">' + md5sum + '</hash>', photon_metalink ])

def set_invalid_sha1(utils):
    photon_metalink = os.path.join(utils.config['repo_path'], metalink_file_path)
    sha1sum = 'ac0a11d67d46f7c629e22714167b8fc3dc2f8e53'
    utils.run([ 'sed', '-i', '-e' ,'/<verification>/a \    <hash type="sha1">' + sha1sum + '</hash>', photon_metalink ])

def set_invalid_sha256(utils):
    photon_metalink = os.path.join(utils.config['repo_path'], metalink_file_path)
    sha256sum = '33853d3329a70ef3e6aab37e31cd03312e66ddc7db20a5f82f06b51ea445dc63'
    utils.run([ 'sed', '-i', '-e' ,'/<verification>/a \    <hash type="sha256">' + sha256sum + '</hash>', photon_metalink ])

def set_invalid_sha256_length(utils):
    photon_metalink = os.path.join(utils.config['repo_path'], metalink_file_path)
    sha256sum = 'ac0a11d67d46f7c629e22714167b8fc3dc2f8e53'
    utils.run([ 'sed', '-i', '-e' ,'/<verification>/a \    <hash type="sha256">' + sha256sum + '</hash>', photon_metalink ])

def set_invalid_sha1_length(utils):
    photon_metalink = os.path.join(utils.config['repo_path'], metalink_file_path)
    sha1sum = 'f894a23a50e757b8aae25596ceb04777'
    utils.run([ 'sed', '-i', '-e' ,'/<verification>/a \    <hash type="sha1">' + sha1sum + '</hash>', photon_metalink ])

#uncomment metalink from repo file, so we have both url and metalink
def test_with_metalink_and_url(utils):
    set_metalink(utils, True)
    set_baseurl(utils, True)
    set_sha1(utils, True)
    set_sha256(utils, False)
    set_md5(utils, False)
    ret = utils.run([ 'tdnf', 'makecache'])
    assert (ret['retval'] == 0)
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)

    utils.run([ 'tdnf', 'install', '-y', '--nogpgcheck', pkgname ])
    assert(utils.check_package(pkgname) == True)

#comment out baseurl
def test_metalink_without_baseurl(utils):
    set_baseurl(utils, False)
    set_metalink(utils, True)
    set_sha256(utils, True)
    set_sha1(utils, False)
    set_md5(utils, False)
    ret = utils.run([ 'tdnf', 'makecache'])
    assert (ret['retval'] == 0)
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)

    utils.run([ 'tdnf', 'install', '-y', '--nogpgcheck', pkgname ])
    assert(utils.check_package(pkgname) == True)

#comment out metalink
#uncomment baseurl
def test_url_without_metalink(utils):
    set_metalink(utils, False)
    set_baseurl(utils, True)
    set_sha256(utils, False)
    set_sha1(utils, False)
    set_md5(utils, False)
    ret = utils.run([ 'tdnf', 'makecache'])
    assert (ret['retval'] == 0)
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)

    utils.run([ 'tdnf', 'install', '-y', '--nogpgcheck', pkgname ])
    assert(utils.check_package(pkgname) == True)

#comment out both metalink and baseurl
def test_without_url_and_metalink(utils):
    set_metalink(utils, False)
    set_baseurl(utils, False)
    set_sha256(utils, False)
    set_sha1(utils, False)
    set_md5(utils, False)
    ret = utils.run([ 'tdnf', 'makecache' ])
    print(ret['stderr'][0])
    assert(ret['stderr'][0].startswith('Error: Cannot find a valid base URL for repo'))


def test_md5_digest(utils):
    set_metalink(utils, True)
    set_baseurl(utils, False)
    set_md5(utils, True)
    set_sha1(utils, False)
    set_sha256(utils, False)
    ret = utils.run([ 'tdnf', 'makecache'])
    assert (ret['retval'] == 0)
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)

    utils.run([ 'tdnf', 'install', '-y', '--nogpgcheck', pkgname ])
    assert(utils.check_package(pkgname) == True)


def test_sha1_digest(utils):
    set_metalink(utils, True)
    set_baseurl(utils, False)
    set_md5(utils, False)
    set_sha1(utils, True)
    set_sha256(utils, False)
    ret = utils.run([ 'tdnf', 'makecache'])
    assert (ret['retval'] == 0)
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)
    utils.run([ 'tdnf', 'install', '-y', '--nogpgcheck', pkgname ])
    assert(utils.check_package(pkgname) == True)


def test_sha256_digest(utils):
    set_metalink(utils, True)
    set_baseurl(utils, False)
    set_md5(utils, False)
    set_sha1(utils, False)
    set_sha256(utils, True)
    ret = utils.run([ 'tdnf', 'makecache'])
    assert (ret['retval'] == 0)
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)

    utils.run([ 'tdnf', 'install', '-y', '--nogpgcheck', pkgname ])
    assert(utils.check_package(pkgname) == True)

def test_invalid_md5_digest(utils):
    set_metalink(utils, True)
    set_baseurl(utils, False)
    set_md5(utils, False)
    set_sha1(utils, False)
    set_sha256(utils, False)
    set_invalid_md5(utils)
    ret = utils.run([ 'tdnf', 'makecache'])
    assert(ret['stderr'][0].startswith('Error: Validating metalink'))

def test_invalid_sha1_digest(utils):
    set_metalink(utils, True)
    set_baseurl(utils, False)
    set_md5(utils, False)
    set_sha1(utils, False)
    set_sha256(utils, False)
    set_invalid_sha1(utils)
    ret = utils.run([ 'tdnf', 'makecache'])
    assert(ret['stderr'][0].startswith('Error: Validating metalink'))

def test_invalid_sha256_digest(utils):
    set_metalink(utils, True)
    set_baseurl(utils, False)
    set_md5(utils, False)
    set_sha1(utils, False)
    set_sha256(utils, False)
    set_invalid_sha256(utils)
    ret = utils.run([ 'tdnf', 'makecache'])
    assert(ret['stderr'][0].startswith('Error: Validating metalink'))

def test_invalid_sha256_valid_sha1(utils):
    set_metalink(utils, True)
    set_baseurl(utils, False)
    set_md5(utils, False)
    set_sha1(utils, False)
    set_sha256(utils, False)
    set_invalid_sha256_length(utils)
    set_sha1(utils,True)
    ret = utils.run([ 'tdnf', 'makecache'])
    assert (ret['retval'] == 0)
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)

    utils.run([ 'tdnf', 'install', '-y', '--nogpgcheck', pkgname ])
    assert(utils.check_package(pkgname) == True)

def test_invalid_sha1_valid_md5(utils):
    set_metalink(utils, True)
    set_baseurl(utils, False)
    set_md5(utils, False)
    set_sha1(utils, False)
    set_sha256(utils, False)
    set_invalid_sha1_length(utils)
    set_md5(utils,True)
    ret = utils.run([ 'tdnf', 'makecache'])
    assert (ret['retval'] == 0)
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)

    utils.run([ 'tdnf', 'install', '-y', '--nogpgcheck', pkgname ])
    assert(utils.check_package(pkgname) == True)
