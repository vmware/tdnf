#
# Copyright (C) 2020-2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import os
import pytest
import configparser

REPO_ID = 'photon-test'
REPO_FILENAME = 'photon-test.repo'
BASEURL = 'http://localhost:8080/photon-test'
METALINK = 'http://localhost:8080/photon-test/metalink'

metalink_file_path = 'photon-test/metalink'
repomd_file_path = 'photon-test/repodata/repomd.xml'


@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    enable_plugin(utils)
    yield
    teardown_test(utils)


# while exiting, uncomment baseurl and comment metalink
def teardown_test(utils):
    set_baseurl(utils, True)
    set_metalink(utils, False)
    set_md5(utils, False)
    set_sha1(utils, False)
    set_sha256(utils, False)
    set_sha512(utils, False)
    pkgname = utils.config["mulversion_pkgname"]
    utils.run(['tdnf', 'erase', '-y', pkgname])
    disable_plugin(utils)


def set_baseurl(utils, enabled):
    repo_file = os.path.join(utils.tdnf_config.get('main', 'repodir'), REPO_FILENAME)
    repo_config = configparser.ConfigParser()
    repo_config.read(repo_file)
    if enabled:
        repo_config[REPO_ID]['baseurl'] = BASEURL
    else:
        repo_config.remove_option(REPO_ID, 'baseurl')
    with open(repo_file, 'w') as f:
        repo_config.write(f, space_around_delimiters=False)


def set_metalink(utils, enabled):
    repo_file = os.path.join(utils.tdnf_config.get('main', 'repodir'), REPO_FILENAME)
    repo_config = configparser.ConfigParser()
    repo_config.read(repo_file)
    if enabled:
        repo_config[REPO_ID]['metalink'] = METALINK
    else:
        repo_config.remove_option(REPO_ID, 'metalink')
    with open(repo_file, 'w') as f:
        repo_config.write(f, space_around_delimiters=False)


def disable_plugin(utils):
    # write a plugin config file
    conf_file = os.path.join(utils.config['repo_path'], 'tdnf.conf')
    tdnf_config = configparser.ConfigParser()
    tdnf_config.read(conf_file)

    plugin_conf_path = os.path.join(utils.config['repo_path'], 'pluginconf.d')

    tdnf_config['main']['plugins'] = '0'
    with open(conf_file, 'w') as f:
        tdnf_config.write(f, space_around_delimiters=False)

    utils.makedirs(plugin_conf_path)

    plugin_file = os.path.join(plugin_conf_path, 'tdnfmetalink.conf')
    with open(plugin_file, 'w') as f:
        f.write('[main]\nenabled=0\n')


def enable_plugin(utils):
    # write a plugin config file
    conf_file = os.path.join(utils.config['repo_path'], 'tdnf.conf')
    tdnf_config = configparser.ConfigParser()
    tdnf_config.read(conf_file)

    plugin_conf_path = os.path.join(utils.config['repo_path'], 'pluginconf.d')
    plugin_path = utils.config['plugin_path']

    tdnf_config['main']['plugins'] = '1'
    tdnf_config['main']['pluginconfpath'] = plugin_conf_path
    tdnf_config['main']['pluginpath'] = plugin_path
    with open(conf_file, 'w') as f:
        tdnf_config.write(f, space_around_delimiters=False)

    utils.makedirs(plugin_conf_path)

    plugin_file = os.path.join(plugin_conf_path, 'tdnfmetalink.conf')
    with open(plugin_file, 'w') as f:
        f.write('[main]\nenabled=1\n')


def set_md5(utils, enabled):
    photon_metalink = os.path.join(utils.config['repo_path'], metalink_file_path)
    if enabled:
        repomd_file = os.path.join(utils.config['repo_path'], repomd_file_path)
        ret = utils.run(['md5sum', repomd_file])
        md5sum = ret['stdout'][0].split()[0]
        utils.run(['sed', '-i', '-e', r'/<verification>/a \    <hash type="md5">' + md5sum + '</hash>', photon_metalink])
    else:
        utils.run(['sed', '-i', '-e', '/type="md5"/d', photon_metalink])


def set_sha1(utils, enabled):
    photon_metalink = os.path.join(utils.config['repo_path'], metalink_file_path)
    if enabled:
        repomd_file = os.path.join(utils.config['repo_path'], repomd_file_path)
        ret = utils.run(['sha1sum', repomd_file])
        sha1sum = ret['stdout'][0].split()[0]
        utils.run(['sed', '-i', '-e', r'/<verification>/a \    <hash type="sha1">' + sha1sum + '</hash>', photon_metalink])
    else:
        utils.run(['sed', '-i', '-e', '/type="sha1"/d', photon_metalink])


def set_sha256(utils, enabled):
    photon_metalink = os.path.join(utils.config['repo_path'], metalink_file_path)
    if enabled:
        repomd_file = os.path.join(utils.config['repo_path'], repomd_file_path)
        ret = utils.run(['sha256sum', repomd_file])
        sha256sum = ret['stdout'][0].split()[0]
        utils.run(['sed', '-i', '-e', r'/<verification>/a \    <hash type="sha256">' + sha256sum + '</hash>', photon_metalink])
    else:
        utils.run(['sed', '-i', '-e', '/type="sha256"/d', photon_metalink])


def set_sha512(utils, enabled):
    photon_metalink = os.path.join(utils.config['repo_path'], metalink_file_path)
    if enabled:
        repomd_file = os.path.join(utils.config['repo_path'], repomd_file_path)
        ret = utils.run(['sha512sum', repomd_file])
        sha512sum = ret['stdout'][0].split()[0]
        utils.run(['sed', '-i', '-e', r'/<verification>/a \    <hash type="sha512">' + sha512sum + '</hash>', photon_metalink])
    else:
        utils.run(['sed', '-i', '-e', '/type="sha512"/d', photon_metalink])


def set_invalid_md5(utils):
    photon_metalink = os.path.join(utils.config['repo_path'], metalink_file_path)
    md5sum = 'f894a23a50e757b8aae25596ceb04777'
    utils.run(['sed', '-i', '-e', r'/<verification>/a \    <hash type="md5">' + md5sum + '</hash>', photon_metalink])


def set_invalid_sha1(utils):
    photon_metalink = os.path.join(utils.config['repo_path'], metalink_file_path)
    sha1sum = 'ac0a11d67d46f7c629e22714167b8fc3dc2f8e53'
    utils.run(['sed', '-i', '-e', r'/<verification>/a \    <hash type="sha1">' + sha1sum + '</hash>', photon_metalink])


def set_invalid_sha256(utils):
    photon_metalink = os.path.join(utils.config['repo_path'], metalink_file_path)
    sha256sum = '33853d3329a70ef3e6aab37e31cd03312e66ddc7db20a5f82f06b51ea445dc63'
    utils.run(['sed', '-i', '-e', r'/<verification>/a \    <hash type="sha256">' + sha256sum + '</hash>', photon_metalink])


def set_invalid_sha256_length(utils):
    photon_metalink = os.path.join(utils.config['repo_path'], metalink_file_path)
    sha256sum = 'ac0a11d67d46f7c629e22714167b8fc3dc2f8e53'
    utils.run(['sed', '-i', '-e', r'/<verification>/a \    <hash type="sha256">' + sha256sum + '</hash>', photon_metalink])


def set_invalid_sha512_length(utils):
    photon_metalink = os.path.join(utils.config['repo_path'], metalink_file_path)
    sha512sum = 'db7a1cec76acd9b8ca2dbf64ba82718064b41844d55216864094d0dc51bb94e28253a2f637503014f3a743c0a4721790602a006a9674e58a3061fcd60ae7807'
    utils.run(['sed', '-i', '-e', r'/<verification>/a \    <hash type="sha512">' + sha512sum + '</hash>', photon_metalink])


def set_invalid_sha1_length(utils):
    photon_metalink = os.path.join(utils.config['repo_path'], metalink_file_path)
    sha1sum = 'f894a23a50e757b8aae25596ceb04777'
    utils.run(['sed', '-i', '-e', r'/<verification>/a \    <hash type="sha1">' + sha1sum + '</hash>', photon_metalink])


# uncomment metalink from repo file, so we have both url and metalink
def test_with_metalink_and_url(utils):
    set_metalink(utils, True)
    set_baseurl(utils, True)
    set_sha1(utils, True)
    set_sha256(utils, False)
    set_sha512(utils, False)
    set_md5(utils, False)
    ret = utils.run(['tdnf', 'makecache'])
    assert ret['retval'] == 0
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)

    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkgname])
    assert utils.check_package(pkgname)


# comment out baseurl
def test_metalink_without_baseurl(utils):
    set_baseurl(utils, False)
    set_metalink(utils, True)
    set_sha256(utils, False)
    set_sha512(utils, True)
    set_sha1(utils, False)
    set_md5(utils, False)
    ret = utils.run(['tdnf', 'makecache'])
    assert ret['retval'] == 0
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)

    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkgname])
    assert utils.check_package(pkgname)


# comment out metalink
# uncomment baseurl
def test_url_without_metalink(utils):
    set_metalink(utils, False)
    set_baseurl(utils, True)
    set_sha256(utils, False)
    set_sha512(utils, False)
    set_sha1(utils, False)
    set_md5(utils, False)
    ret = utils.run(['tdnf', 'makecache'])
    assert ret['retval'] == 0
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)

    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkgname])
    assert utils.check_package(pkgname)


# comment out both metalink and baseurl
def test_without_url_and_metalink(utils):
    set_metalink(utils, False)
    set_baseurl(utils, False)
    set_sha256(utils, False)
    set_sha512(utils, False)
    set_sha1(utils, False)
    set_md5(utils, False)
    ret = utils.run(['tdnf', 'makecache'])
    print(ret['stderr'][0])
    assert ret['stderr'][0].startswith('Error: Cannot find a valid base URL for repo')


def test_md5_digest(utils):
    set_metalink(utils, True)
    set_baseurl(utils, False)
    set_md5(utils, True)
    set_sha1(utils, False)
    set_sha256(utils, False)
    set_sha512(utils, False)
    ret = utils.run(['tdnf', 'makecache'])
    assert ret['retval'] == 0
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)

    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkgname])
    assert utils.check_package(pkgname)


def test_sha1_digest(utils):
    set_metalink(utils, True)
    set_baseurl(utils, False)
    set_md5(utils, False)
    set_sha1(utils, True)
    set_sha256(utils, False)
    set_sha512(utils, False)
    ret = utils.run(['tdnf', 'makecache'])
    assert ret['retval'] == 0
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)
    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkgname])
    assert utils.check_package(pkgname)


def test_sha256_digest(utils):
    set_metalink(utils, True)
    set_baseurl(utils, False)
    set_md5(utils, False)
    set_sha1(utils, False)
    set_sha256(utils, True)
    set_sha512(utils, False)
    ret = utils.run(['tdnf', 'makecache'])
    assert ret['retval'] == 0
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)

    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkgname])
    assert utils.check_package(pkgname)


def test_sha512_digest(utils):
    set_metalink(utils, True)
    set_baseurl(utils, False)
    set_md5(utils, False)
    set_sha1(utils, False)
    set_sha256(utils, False)
    set_sha512(utils, True)
    ret = utils.run(['tdnf', 'makecache'])
    assert ret['retval'] == 0
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)

    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkgname])
    assert utils.check_package(pkgname)


def test_invalid_md5_digest(utils):
    set_metalink(utils, True)
    set_baseurl(utils, False)
    set_md5(utils, False)
    set_sha1(utils, False)
    set_sha256(utils, False)
    set_sha512(utils, False)
    set_invalid_md5(utils)
    ret = utils.run(['tdnf', 'makecache'])
    assert ret['stderr'][0].startswith('Error: Validating metalink')
    cache_dir = utils.tdnf_config.get('main', 'cachedir')
    tmp_dir = os.path.join(cache_dir, 'photon-test/tmp')
    assert not os.path.isdir(tmp_dir)


def test_invalid_sha1_digest(utils):
    set_metalink(utils, True)
    set_baseurl(utils, False)
    set_md5(utils, False)
    set_sha1(utils, False)
    set_sha256(utils, False)
    set_sha512(utils, False)
    set_invalid_sha1(utils)
    ret = utils.run(['tdnf', 'makecache'])
    assert ret['stderr'][0].startswith('Error: Validating metalink')
    cache_dir = utils.tdnf_config.get('main', 'cachedir')
    tmp_dir = os.path.join(cache_dir, 'photon-test/tmp')
    assert not os.path.isdir(tmp_dir)


def test_invalid_sha256_digest(utils):
    set_metalink(utils, True)
    set_baseurl(utils, False)
    set_md5(utils, False)
    set_sha1(utils, False)
    set_sha256(utils, False)
    set_sha512(utils, False)
    set_invalid_sha256(utils)
    ret = utils.run(['tdnf', 'makecache'])
    assert ret['stderr'][0].startswith('Error: Validating metalink')
    cache_dir = utils.tdnf_config.get('main', 'cachedir')
    tmp_dir = os.path.join(cache_dir, 'photon-test/tmp')
    assert not os.path.isdir(tmp_dir)


def test_invalid_sha256_valid_sha1(utils):
    set_metalink(utils, True)
    set_baseurl(utils, False)
    set_md5(utils, False)
    set_sha1(utils, False)
    set_sha256(utils, False)
    set_sha512(utils, False)
    set_invalid_sha256_length(utils)
    set_sha1(utils, True)
    ret = utils.run(['tdnf', 'makecache'])
    assert ret['retval'] == 0
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)

    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkgname])
    assert utils.check_package(pkgname)


def test_invalid_sha512_valid_sha1(utils):
    set_metalink(utils, True)
    set_baseurl(utils, False)
    set_md5(utils, False)
    set_sha1(utils, False)
    set_sha256(utils, False)
    set_sha512(utils, False)
    set_invalid_sha512_length(utils)
    set_sha1(utils, True)
    ret = utils.run(['tdnf', 'makecache'])
    assert ret['retval'] == 0
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)

    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkgname])
    assert utils.check_package(pkgname)


def test_invalid_sha1_valid_md5(utils):
    set_metalink(utils, True)
    set_baseurl(utils, False)
    set_md5(utils, False)
    set_sha1(utils, False)
    set_sha256(utils, False)
    set_sha512(utils, False)
    set_invalid_sha1_length(utils)
    set_md5(utils, True)
    ret = utils.run(['tdnf', 'makecache'])
    assert ret['retval'] == 0
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)

    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkgname])
    assert utils.check_package(pkgname)
