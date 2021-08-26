#
# Copyright (C) 2021 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#
#   Author: Oliver Kurth <okurth@vmware.com>

import os
import pytest
import errno
import shutil

TESTREPO='photon-test'

@pytest.fixture(scope='function', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)

def teardown_test(utils):
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)

    dirname = os.path.join(utils.config['repo_path'], 'minversions.d')
    if os.path.isdir(dirname):
        shutil.rmtree(dirname)

    utils.tdnf_config.remove_option('main', 'minversions')
    filename = os.path.join(utils.config['repo_path'], 'tdnf.conf')
    with open(filename, 'w') as f:
        utils.tdnf_config.write(f, space_around_delimiters=False)

# helper to create directory tree without complains when it exists:
def makedirs(d):
    try:
        os.makedirs(d)
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

def set_minversions_conf(utils, value):
    utils.tdnf_config['main']['minversions'] = value
    filename = os.path.join(utils.config['repo_path'], 'tdnf.conf')
    with open(filename, 'w') as f:
        utils.tdnf_config.write(f, space_around_delimiters=False)

def set_minversions_file(utils, value):
    utils.tdnf_config['main']['minversions'] = value
    dirname = os.path.join(utils.config['repo_path'], 'minversions.d')
    makedirs(dirname)
    filename = os.path.join(dirname, 'test.conf')
    with open(filename, 'w') as f:
        f.write(value)

def test_minversions_conf(utils):
    pkgname = utils.config["mulversion_pkgname"]
    set_minversions_conf(utils, "{}={}".format(pkgname, utils.config["mulversion_higher"]))
    utils.install_package(pkgname)
    ret = utils.run(['tdnf', '-y', '--nogpgcheck', 'downgrade', pkgname])
    print (ret)
    assert(ret['retval'] == 1033)

def test_minversions_conf_multiple(utils):
    pkgname = utils.config["mulversion_pkgname"]
    set_minversions_conf(utils, "bogus=1.2.3 {}={}".format(pkgname, utils.config["mulversion_higher"]))
    utils.install_package(pkgname)
    ret = utils.run(['tdnf', '-y', '--nogpgcheck', 'downgrade', pkgname])
    print (ret)
    assert(ret['retval'] == 1033)

def test_minversions_file(utils):
    pkgname = utils.config["mulversion_pkgname"]
    set_minversions_file(utils, "{}={}".format(pkgname, utils.config["mulversion_higher"]))
    utils.install_package(pkgname)
    ret = utils.run(['tdnf', '-y', '--nogpgcheck', 'downgrade', pkgname])
    print (ret)
    assert(ret['retval'] == 1033)

def test_minversions_file_multiple(utils):
    pkgname = utils.config["mulversion_pkgname"]
    set_minversions_file(utils, "bogus=1.2.3\n{}={}".format(pkgname, utils.config["mulversion_higher"]))
    utils.install_package(pkgname)
    ret = utils.run(['tdnf', '-y', '--nogpgcheck', 'downgrade', pkgname])
    print (ret)
    assert(ret['retval'] == 1033)


