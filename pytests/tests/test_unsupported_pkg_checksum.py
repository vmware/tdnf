#
# Copyright (C) 2026 Broadcom, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

"""
Packages whose repository checksum type tdnf cannot verify (anything other
than SHA-1, SHA-256 and SHA-512) must never be installed unverified, but
must not break queries of the repository either.
"""

import glob
import os
import platform
import shutil
import subprocess
import tempfile
import pytest

ARCH = platform.machine()
PKGNAME = 'tdnf-test-one'
REPONAME = 'unsupported-checksum'
ERROR_TDNF_INVALID_REPO_FILE = 1004


def _find_rpm(utils, pkgname):
    pattern = os.path.join(utils.config['repo_path'], 'build', 'RPMS', ARCH,
                           f'{pkgname}-*.rpm')
    matches = glob.glob(pattern)
    assert matches, f'No RPM found for {pkgname}'
    return matches[0]


@pytest.fixture(scope='module', params=['sha224', 'sha384'])
def env(utils, request):
    workdir = tempfile.mkdtemp(prefix='tdnf-unsupported-checksum-')
    repodir = os.path.join(workdir, 'repo')
    reposdir = os.path.join(workdir, 'yum.repos.d')
    cachedir = os.path.join(workdir, 'cache')
    os.makedirs(repodir)
    os.makedirs(reposdir)
    os.makedirs(cachedir)

    shutil.copy(_find_rpm(utils, PKGNAME), repodir)
    subprocess.run(['createrepo_c', '--quiet',
                    '--checksum', request.param,
                    '--repomd-checksum', 'sha256',
                    repodir], check=True)

    conf = os.path.join(workdir, 'tdnf.conf')
    with open(conf, 'w') as f:
        f.write(f'[main]\ngpgcheck=0\nrepodir={reposdir}\ncachedir={cachedir}\n')
    with open(os.path.join(reposdir, f'{REPONAME}.repo'), 'w') as f:
        f.write(f'[{REPONAME}]\nname={REPONAME}\nbaseurl=file://{repodir}\n'
                'enabled=1\ngpgcheck=0\n')

    yield {'conf': conf, 'cachedir': cachedir}

    shutil.rmtree(workdir)


@pytest.fixture(scope='function', autouse=True)
def setup_test(utils):
    utils.erase_package(PKGNAME)
    yield
    utils.erase_package(PKGNAME)


def test_info_with_unsupported_checksum(utils, env):
    ret = utils.run(['tdnf', '-c', env['conf'], 'info', PKGNAME])
    assert ret['retval'] == 0
    assert any(PKGNAME in line for line in ret['stdout'])


def test_install_with_unsupported_checksum(utils, env):
    ret = utils.run(['tdnf', '-c', env['conf'], 'install', '-y', PKGNAME])
    assert ret['retval'] == ERROR_TDNF_INVALID_REPO_FILE
    assert any('Unsupported repository checksum type' in line for line in ret['stderr'])
    assert not utils.check_package(PKGNAME)
    # rejected before the package was downloaded
    rpms = glob.glob(os.path.join(env['cachedir'], '**', '*.rpm'), recursive=True)
    assert rpms == []


def test_provides_with_unsupported_checksum(utils, env):
    ret = utils.run(['tdnf', '-c', env['conf'], 'provides', PKGNAME])
    assert ret['retval'] == 0
    assert any(PKGNAME in line for line in ret['stdout'])


# the transaction can still be shown, without installing anything
def test_install_assumeno_with_unsupported_checksum(utils, env):
    ret = utils.run(['tdnf', '-c', env['conf'], 'install', '--assumeno', PKGNAME])
    assert not any('Unsupported repository checksum' in line for line in ret['stderr'])
    assert any(PKGNAME in line for line in ret['stdout'])
    assert not utils.check_package(PKGNAME)
