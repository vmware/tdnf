#
# Copyright (C) 2019 - 2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import os
import subprocess
import pytest
import shutil


WORKDIR = '/root/test_tsflags'
TEST_CONF_FILE = 'tdnf.conf'
TEST_CONF_PATH = os.path.join(WORKDIR, TEST_CONF_FILE)


@pytest.fixture(scope='function', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    packages = ["tdnf-bad-pre", utils.config["mulversion_pkgname"], "tdnf-test-doc"]
    utils.run(['tdnf', 'erase', '-y'] + packages)

    try:
        os.remove(TEST_CONF_PATH)
    except OSError:
        pass


# Verify that the package is really bad:
def test_install_normal(utils):
    pkgname = 'tdnf-bad-pre'
    ret = utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkgname])
    assert ret['retval'] == 1525


def test_install_noscripts(utils):
    pkgname = 'tdnf-bad-pre'
    ret = utils.run(['tdnf', 'install', '-y', '--nogpgcheck',
                     '--setopt=tsflags=noscripts', pkgname])
    assert ret['retval'] == 0


def test_install_noscripts_nodocs(utils):
    pkgname = 'tdnf-bad-pre'
    ret = utils.run(['tdnf', 'install', '-y', '--nogpgcheck',
                     '--setopt=tsflags=noscripts', '--setopt=tsflags=nodocs', pkgname])
    assert ret['retval'] == 0


def test_install_justdb(utils):
    pkgname = utils.config["mulversion_pkgname"]
    ret = utils.run(['tdnf', 'install', '-y', '--nogpgcheck',
                     '--setopt=tsflags=justdb', pkgname])
    assert ret['retval'] == 0

    # package should look like it's installed, but no files should be installed
    assert utils.check_package(pkgname)
    process = subprocess.run(['rpm', '-ql', pkgname], stdout=subprocess.PIPE, text=True)
    for f in process.stdout.split():
        assert not os.path.exists(f)


# opposite of --justdb
def test_install_nodb(utils):
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)

    ret = utils.run(['tdnf', 'install', '-y', '--nogpgcheck',
                     '--setopt=tsflags=nodb', pkgname])
    assert ret['retval'] == 0

    # package should appear to not be installed, but files should be there:
    assert not utils.check_package(pkgname)
    process = subprocess.run(['tdnf', 'repoquery', '--list', pkgname], stdout=subprocess.PIPE, text=True)
    for f in process.stdout.split():
        if f.startswith("/"):
            assert os.path.exists(f)


# opposite of --justdb
def test_install_nodocs(utils):
    pkgname = "tdnf-test-doc"
    utils.erase_package(pkgname)

    ret = utils.run(['tdnf', 'install', '-y', '--nogpgcheck',
                     '--setopt=tsflags=nodocs', pkgname])
    assert ret['retval'] == 0

    # make sure packe is installed
    assert utils.check_package(pkgname)
    # but no doc file
    assert not os.path.exists("/usr/share/doc/tdnf-test-doc/README")


def test_install_multiple(utils):
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)

    ret = utils.run(['tdnf', 'install', '-y', '--nogpgcheck',
                     '--setopt=tsflags=nodb nodocs', pkgname])
    assert ret['retval'] == 0

    # package should appear to not be installed, but files should be there:
    assert not utils.check_package(pkgname)
    process = subprocess.run(['tdnf', 'repoquery', '--list', pkgname], stdout=subprocess.PIPE, text=True)
    for f in process.stdout.split():
        if f.startswith("/"):
            assert os.path.exists(f)

    # no doc file
    assert not os.path.exists("/usr/share/doc/tdnf-test-doc/README")


def test_install_multiple_from_config(utils):
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)

    tdnf_conf = os.path.join(utils.config['repo_path'], 'tdnf.conf')

    utils.makedirs(WORKDIR)
    shutil.copy(tdnf_conf, TEST_CONF_PATH)
    utils.edit_config({'tsflags': "nodb nodocs"}, section='main', filename=TEST_CONF_PATH)

    ret = utils.run(['tdnf', 'install', '-y', '--nogpgcheck',
                     '-c', TEST_CONF_PATH, pkgname])
    assert ret['retval'] == 0

    # package should appear to not be installed, but files should be there:
    assert not utils.check_package(pkgname)
    process = subprocess.run(['tdnf', 'repoquery', '--list', pkgname], stdout=subprocess.PIPE, text=True)
    for f in process.stdout.split():
        if f.startswith("/"):
            assert os.path.exists(f)

    # no doc file
    assert not os.path.exists("/usr/share/doc/tdnf-test-doc/README")

    # do this again, but override config settings to "normal":
    ret = utils.run(['tdnf', 'install', '-y', '--nogpgcheck',
                     '-c', TEST_CONF_PATH, '--setopt=tsflags=', pkgname])
    assert ret['retval'] == 0

    assert utils.check_package(pkgname)

    # check all files exist - that includes doc files:
    process = subprocess.run(['tdnf', 'repoquery', '--list', pkgname], stdout=subprocess.PIPE, text=True)
    for f in process.stdout.split():
        if f.startswith("/"):
            assert os.path.exists(f)
