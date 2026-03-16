#
# Copyright (C) 2026 Broadcom, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.

import pytest


@pytest.fixture(scope='function', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    pkgs = [
        utils.config['sglversion_pkgname'],
        'tdnf-test-cleanreq-leaf1',
        'tdnf-test-cleanreq-required',
    ]
    utils.run('tdnf remove -y ' + ' '.join(pkgs))


# --urls must print a URL and not install the package
def test_urls_basic(utils):
    pkgname = utils.config['sglversion_pkgname']
    utils.erase_package(pkgname)

    ret = utils.run(['tdnf', 'install', '-y', '--urls', pkgname])
    assert ret['retval'] == 0
    assert not utils.check_package(pkgname)

    output = '\n'.join(ret['stdout'])
    assert pkgname in output
    assert output.strip().endswith('.rpm')


# each printed line must be a valid URL (starts with a known scheme or '/')
def test_urls_format(utils):
    pkgname = utils.config['sglversion_pkgname']
    utils.erase_package(pkgname)

    ret = utils.run(['tdnf', 'install', '-y', '--urls', pkgname])
    assert ret['retval'] == 0

    for line in ret['stdout']:
        if line.strip():
            assert (line.startswith('http://') or line.startswith('https://') or line.startswith('ftp://') or line.startswith('file://') or line.startswith('/')), \
                f"unexpected URL format: {line}"


# an uninstalled requirement must also appear in the URL list (default deps)
def test_urls_includes_requires(utils):
    pkgname = 'tdnf-test-cleanreq-leaf1'
    pkgname_req = 'tdnf-test-cleanreq-required'

    utils.erase_package(pkgname_req)
    assert not utils.check_package(pkgname_req)

    ret = utils.run(['tdnf', 'install', '-y', '--urls', pkgname])
    assert ret['retval'] == 0
    assert not utils.check_package(pkgname)
    assert not utils.check_package(pkgname_req)

    output = '\n'.join(ret['stdout'])
    assert pkgname_req in output


# --alldeps: an already-installed requirement must still appear in the URL list
def test_urls_alldeps(utils):
    pkgname = 'tdnf-test-cleanreq-leaf1'
    pkgname_req = 'tdnf-test-cleanreq-required'

    utils.install_package(pkgname_req)
    assert utils.check_package(pkgname_req)

    ret = utils.run(['tdnf', 'install', '-y', '--urls', '--alldeps', pkgname])
    assert ret['retval'] == 0
    assert not utils.check_package(pkgname)

    output = '\n'.join(ret['stdout'])
    assert pkgname_req in output


# --nodeps: the requirement must NOT appear in the URL list
def test_urls_nodeps(utils):
    pkgname = 'tdnf-test-cleanreq-leaf1'
    pkgname_req = 'tdnf-test-cleanreq-required'

    utils.erase_package(pkgname_req)
    assert not utils.check_package(pkgname_req)

    ret = utils.run(['tdnf', 'install', '-y', '--urls', '--nodeps', pkgname])
    assert ret['retval'] == 0
    assert not utils.check_package(pkgname)

    output = '\n'.join(ret['stdout'])
    assert pkgname_req not in output


# --alldeps without --urls or --downloadonly must fail
def test_urls_alldeps_requires_urls_or_downloadonly(utils):
    pkgname = utils.config['sglversion_pkgname']
    ret = utils.run(['tdnf', 'install', '-y', '--alldeps', pkgname])
    assert ret['retval'] != 0


# --nodeps without --urls or --downloadonly must fail
def test_urls_nodeps_requires_urls_or_downloadonly(utils):
    pkgname = utils.config['sglversion_pkgname']
    ret = utils.run(['tdnf', 'install', '-y', '--nodeps', pkgname])
    assert ret['retval'] != 0
