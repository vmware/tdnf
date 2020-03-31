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

@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)

#while exiting, uncomment baseurl and comment metalink
def teardown_test(utils):
    tdnf_repo = os.path.join(utils.tdnf_config.get('main', 'repodir'), 'photon-test.repo')
    set_baseurl(utils, True)
    set_metalink(utils, False)

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

#uncomment metalink from repo file, so we have both url and metalink
def test_with_metalink_and_url(utils):
    set_metalink(utils, True)
    set_baseurl(utils, True)
    ret = utils.run([ 'tdnf', 'repolist'])
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)

    utils.run([ 'tdnf', 'install', '-y', '--nogpgcheck', pkgname ])
    assert(utils.check_package(pkgname) == True)

#comment out baseurl
def test_metalink_without_baseurl(utils):
    set_baseurl(utils, False)
    set_metalink(utils, True)
    ret = utils.run([ 'tdnf', 'repolist'])
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)

    utils.run([ 'tdnf', 'install', '-y', '--nogpgcheck', pkgname ])
    assert(utils.check_package(pkgname) == True)

#comment out metalink
#uncomment baseurl
def test_url_without_metalink(utils):
    set_metalink(utils, False)
    set_baseurl(utils, True)
    ret = utils.run([ 'tdnf', 'repolist'])
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)

    utils.run([ 'tdnf', 'install', '-y', '--nogpgcheck', pkgname ])
    assert(utils.check_package(pkgname) == True)

#comment out both metalink and baseurl
def test_without_url_and_metalink(utils):
    set_metalink(utils, False)
    set_baseurl(utils, False)
    ret = utils.run([ 'tdnf', 'repolist' ])
    print(ret['stderr'][0])
    assert(ret['stderr'][0].startswith('Error: Cannot find a valid base URL for repo'))
