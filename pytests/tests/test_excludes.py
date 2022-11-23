#
# Copyright (C) 2019 - 2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import pytest
import os
import shutil


@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    for pkg in ("mulversion_pkgname", "requiring_package", "required_package"):
        pkgname = utils.config[pkg]
        utils.run(['tdnf', 'erase', '-y', pkgname])

    dirname = os.path.join(utils.config['repo_path'], 'minversions.d')
    if os.path.isdir(dirname):
        shutil.rmtree(dirname)

    utils.tdnf_config.remove_option('main', 'minversions')
    filename = os.path.join(utils.config['repo_path'], 'tdnf.conf')
    with open(filename, 'w') as f:
        utils.tdnf_config.write(f, space_around_delimiters=False)


def set_minversions_file(utils, value):
    dirname = os.path.join(utils.config['repo_path'], 'minversions.d')
    utils.makedirs(dirname)
    filename = os.path.join(dirname, 'test.conf')
    with open(filename, 'w') as f:
        f.write(value)


# specifying the version should not override the exclude (negative test)
def test_install_package_with_version_suffix(utils):
    pkgname = utils.config["mulversion_pkgname"]
    pkgversion = utils.config["mulversion_lower"]
    utils.erase_package(pkgname)

    utils.run(['tdnf', 'install', '--exclude=', pkgname, '-y', '--nogpgcheck', pkgname + '-' + pkgversion])
    assert not utils.check_package(pkgname)


# basic test (negative test)
def test_install_package_without_version_suffix(utils):
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)

    utils.run(['tdnf', 'install', '--exclude=', pkgname, '-y', '--nogpgcheck', pkgname])
    assert not utils.check_package(pkgname)


# excluded dependency (negative test)
def test_install_package_with_excluded_dependency(utils):
    pkgname = utils.config["requiring_package"]
    pkgname_required = utils.config["required_package"]
    utils.erase_package(pkgname)
    utils.erase_package(pkgname_required)

    utils.run(['tdnf', 'install', '--exclude=', pkgname_required, '-y', '--nogpgcheck', pkgname])
    assert not utils.check_package(pkgname)


# an update should skip an excluded package (negative test)
def test_update_package(utils):
    pkgname = utils.config["mulversion_pkgname"]
    pkgversion1 = utils.config["mulversion_lower"]
    pkgversion2 = utils.config["mulversion_higher"]

    if '-' in pkgversion1:
        pkgversion1 = pkgversion1.split('-')[0]
    if '-' in pkgversion2:
        pkgversion2 = pkgversion2.split('-')[0]

    utils.erase_package(pkgname)

    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkgname + '-' + pkgversion1])
    assert utils.check_package(pkgname, pkgversion1)

    utils.run(['tdnf', 'update', '--exclude=', pkgname, '-y', '--nogpgcheck', pkgname + '-' + pkgversion2])
    assert not utils.check_package(pkgname, pkgversion2)
    assert utils.check_package(pkgname, pkgversion1)


# removing an excluded package should fail (dnf behavior) (negative test)
def test_remove_package(utils):
    pkgname = utils.config["mulversion_pkgname"]
    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkgname])
    utils.run(['tdnf', 'remove', '--exclude=', pkgname, '-y', '--nogpgcheck', pkgname])
    # package should still be there
    assert utils.check_package(pkgname)


# test for issue #367
def test_with_minversion_existing(utils):
    mverpkg = utils.config["sglversion_pkgname"]
    set_minversions_file(utils, mverpkg + "=1.0.1-2\n")

    pkgname = utils.config["mulversion_pkgname"]
    pkgversion1 = utils.config["mulversion_lower"]
    pkgversion2 = utils.config["mulversion_higher"]

    if '-' in pkgversion1:
        pkgversion1 = pkgversion1.split('-')[0]
    if '-' in pkgversion2:
        pkgversion2 = pkgversion2.split('-')[0]

    utils.erase_package(pkgname)

    utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkgname + '-' + pkgversion1])
    assert utils.check_package(pkgname, pkgversion1)

    utils.run(['tdnf', 'update', '--exclude=', '-y', '--nogpgcheck', pkgname + '-' + pkgversion2])
    assert not utils.check_package(pkgname, pkgversion2)
    assert utils.check_package(pkgname, pkgversion1)
