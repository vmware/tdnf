#
# Copyright (C) 2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#
#   Author: Oliver Kurth <okurth@vmware.com>

import os
import shutil
import pytest


CONFDIR = '/tmp/cleanreq'


@pytest.fixture(scope='function', autouse=True)
def setup_test(utils):
    utils.makedirs(CONFDIR)
    yield
    teardown_test(utils)


def teardown_test(utils):
    utils.erase_package('tdnf-test-cleanreq-leaf1')
    utils.erase_package('tdnf-test-cleanreq-leaf2')
    utils.erase_package('tdnf-test-cleanreq-required')
    shutil.rmtree(CONFDIR)


def generate_config_cleanreq(utils, newconfig, value):
    orig_conffile = os.path.join(utils.config['repo_path'], 'tdnf.conf')

    with open(orig_conffile, 'r') as fin:
        with open(newconfig, 'w') as fout:
            for line in fin:
                if not line.startswith('clean_requirements_on_remove'):
                    fout.write(line)
                else:
                    fout.write('clean_requirements_on_remove={}\n'.format('1' if value else '0'))

    with open(newconfig, 'r') as fin:
        for line in fin:
            print(line)


# leaf1 pulls in a dependency, this should be removed when leaf1 is
# autoremoved
def test_autoremove(utils):
    pkgname = 'tdnf-test-cleanreq-leaf1'
    pkgname_req = 'tdnf-test-cleanreq-required'
    utils.install_package(pkgname)

    assert(utils.check_package(pkgname_req))

    utils.run(['tdnf', '-y', 'autoremove', pkgname])

    assert(not utils.check_package(pkgname))
    # actual test:
    assert(not utils.check_package(pkgname_req))


# When the required package is installed first, removing the
# leaf should not uninstall it.
def test_autoremove_req_is_user_installed(utils):
    pkgname = 'tdnf-test-cleanreq-leaf1'
    pkgname_req = 'tdnf-test-cleanreq-required'
    utils.install_package(pkgname_req)
    utils.install_package(pkgname)

    assert(utils.check_package(pkgname_req))
    assert(utils.check_package(pkgname))

    utils.run(['tdnf', '-y', 'autoremove', pkgname])

    assert(not utils.check_package(pkgname))
    # actual test:
    assert(utils.check_package(pkgname_req))


# When the required package has been installed as a dependency
# an attempt to install it (although it already is) should
# mark it has user installed, and removing leaf should not uninstall it.
def test_autoremove_req_is_user_installed2(utils):
    pkgname = 'tdnf-test-cleanreq-leaf1'
    pkgname_req = 'tdnf-test-cleanreq-required'
    utils.install_package(pkgname)
    utils.install_package(pkgname_req)

    assert(utils.check_package(pkgname))
    assert(utils.check_package(pkgname_req))

    utils.run(['tdnf', '-y', 'autoremove', pkgname])

    assert(not utils.check_package(pkgname))
    # actual test:
    assert(utils.check_package(pkgname_req))


# leaf1 pulls in a dependency. leaf2 has the same dependency.
# The required package should not be removed when leaf1 is uninstalled
# only when leaf2 is uninstalled too.
def test_autoremove2(utils):
    pkgname = 'tdnf-test-cleanreq-leaf1'
    pkgname2 = 'tdnf-test-cleanreq-leaf2'
    pkgname_req = 'tdnf-test-cleanreq-required'

    utils.install_package(pkgname)
    utils.install_package(pkgname2)

    utils.run(['tdnf', '-y', 'autoremove', pkgname])
    assert(not utils.check_package(pkgname))

    # actual test:
    assert(utils.check_package(pkgname_req))

    utils.run(['tdnf', '-y', 'autoremove', pkgname2])

    # actual test:
    assert(not utils.check_package(pkgname_req))


# like autoremove, but in config file
def test_autoremove_conf_true(utils):
    conffile = os.path.join(CONFDIR, 'tdnf.conf')
    generate_config_cleanreq(utils, conffile, True)

    pkgname = 'tdnf-test-cleanreq-leaf1'
    pkgname_req = 'tdnf-test-cleanreq-required'

    utils.install_package(pkgname)
    assert(utils.check_package(pkgname_req))

    utils.run(['tdnf', '-y', '-c', conffile, 'remove', pkgname])

    assert(not utils.check_package(pkgname))
    # actual test:
    assert(not utils.check_package(pkgname_req))


# autoremove disabled in config file
def test_autoremove_conf_false(utils):
    conffile = os.path.join(CONFDIR, 'tdnf.conf')
    generate_config_cleanreq(utils, conffile, False)

    pkgname = 'tdnf-test-cleanreq-leaf1'
    pkgname_req = 'tdnf-test-cleanreq-required'

    utils.install_package(pkgname)
    assert(utils.check_package(pkgname_req))

    utils.run(['tdnf', '-y', '-c', conffile, 'remove', pkgname])

    assert(not utils.check_package(pkgname))
    # actual test:
    assert(utils.check_package(pkgname_req))


# autoremove enabled in config file, but --noautoremove in cmd line
def test_autoremove_conf_noautoremove(utils):
    conffile = os.path.join(CONFDIR, 'tdnf.conf')
    generate_config_cleanreq(utils, conffile, True)

    pkgname = 'tdnf-test-cleanreq-leaf1'
    pkgname_req = 'tdnf-test-cleanreq-required'

    utils.install_package(pkgname)
    assert(utils.check_package(pkgname_req))

    utils.run(['tdnf', '-y', '-c', conffile, '--noautoremove', 'remove', pkgname])

    assert(not utils.check_package(pkgname))
    # actual test:
    assert(utils.check_package(pkgname_req))


# autoremove disabled in config file, 'autoremove' should still
# do the clean up
def test_autoremove_conf_false_autoremove(utils):
    conffile = os.path.join(CONFDIR, 'tdnf.conf')
    generate_config_cleanreq(utils, conffile, False)

    pkgname = 'tdnf-test-cleanreq-leaf1'
    pkgname_req = 'tdnf-test-cleanreq-required'

    utils.install_package(pkgname)
    assert(utils.check_package(pkgname_req))

    utils.run(['tdnf', '-y', '-c', conffile, 'autoremove', pkgname])

    assert(not utils.check_package(pkgname))
    # actual test:
    assert(not utils.check_package(pkgname_req))
