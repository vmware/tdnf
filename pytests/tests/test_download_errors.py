#
# Copyright (C) 2026 Broadcom, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import os
import shutil
import pytest

WORKDIR = '/tmp/test_download_errors/workdir'
BASEDIR = os.path.dirname(WORKDIR)
REPOFILENAME = 'download_errors.repo'
REPONAME = 'download-errors-repo'

# A URL that resolves but points to a non-existent path -> HTTP 404
BAD_HTTP_URL = 'http://localhost:8080/doesntexist'
# A URL that does not resolve at all -> curl connection error
BAD_CURL_URL = 'http://localhost:19999/doesntexist'
# The working test repo URL
GOOD_HTTP_URL = 'http://localhost:8080/photon-test'


@pytest.fixture(scope='function', autouse=True)
def setup_test(utils):
    os.makedirs(WORKDIR, exist_ok=True)
    yield
    teardown_test(utils)


def teardown_test(utils):
    pkgname = utils.config['mulversion_pkgname']
    utils.erase_package(pkgname)
    if os.path.isdir(BASEDIR):
        shutil.rmtree(BASEDIR)
    repofile = os.path.join(utils.config['repo_path'], 'yum.repos.d', REPOFILENAME)
    if os.path.isfile(repofile):
        os.remove(repofile)


def repofile_path(utils):
    return os.path.join(utils.config['repo_path'], 'yum.repos.d', REPOFILENAME)


# HTTP 404: error message must contain the full URL and the HTTP status code
def test_http_error_contains_url(utils):
    utils.create_repoconf(repofile_path(utils), BAD_HTTP_URL, REPONAME)

    ret = utils.run(['tdnf',
                     '--disablerepo=*', f'--enablerepo={REPONAME}',
                     'makecache'],
                    cwd=WORKDIR)
    assert ret['retval'] != 0
    stderr = '\n'.join(ret['stderr'])
    assert BAD_HTTP_URL in stderr
    assert f'Error: 404 when downloading {BAD_HTTP_URL}' in stderr


# curl connection failure: error message must contain the full URL and curl error
def test_curl_error_contains_url(utils):
    utils.create_repoconf(repofile_path(utils), BAD_CURL_URL, REPONAME)

    ret = utils.run(['tdnf',
                     '--disablerepo=*', f'--enablerepo={REPONAME}',
                     'makecache'],
                    cwd=WORKDIR)
    assert ret['retval'] != 0
    stderr = '\n'.join(ret['stderr'])
    assert BAD_CURL_URL in stderr
    assert f'Error: failed to download {BAD_CURL_URL}' in stderr


# Multi-URL fallback: the failed URL must appear in the warning, and the
# operation must ultimately succeed using the second (good) URL
def test_multi_baseurl_fallback_warning_contains_url(utils):
    baseurls = f'{BAD_HTTP_URL} {GOOD_HTTP_URL}'
    utils.create_repoconf(repofile_path(utils), baseurls, REPONAME)

    ret = utils.run(['tdnf',
                     '--disablerepo=*', f'--enablerepo={REPONAME}',
                     'makecache'],
                    cwd=WORKDIR)
    assert ret['retval'] == 0
    stderr = '\n'.join(ret['stderr'])
    assert BAD_HTTP_URL in stderr
    assert any(
        line.startswith(f'Warning: failed to download {BAD_HTTP_URL}') and
        line.endswith(', trying next base URL')
        for line in ret['stderr']
    )


# Multi-URL fallback for package download: the failed URL must appear in
# the warning, and the install must succeed using the second (good) URL
def test_multi_baseurl_pkg_download_warning_contains_url(utils):
    pkgname = utils.config['mulversion_pkgname']
    utils.erase_package(pkgname)

    baseurls = f'{BAD_HTTP_URL} {GOOD_HTTP_URL}'
    utils.create_repoconf(repofile_path(utils), baseurls, REPONAME)

    # prime the cache with a working URL first
    ret = utils.run(['tdnf',
                     '--disablerepo=*', f'--enablerepo={REPONAME}',
                     '--setopt={}.baseurl={}'.format(REPONAME, GOOD_HTTP_URL),
                     'makecache'],
                    cwd=WORKDIR)
    assert ret['retval'] == 0

    ret = utils.run(['tdnf', '-y', '--nogpgcheck',
                     '--disablerepo=*', f'--enablerepo={REPONAME}',
                     'install', pkgname],
                    cwd=WORKDIR)
    assert ret['retval'] == 0
    assert utils.check_package(pkgname)
    assert BAD_HTTP_URL in '\n'.join(ret['stderr'])
    assert any(
        line.startswith(f'Warning: failed to download {BAD_HTTP_URL}') and
        line.endswith(', trying next base URL')
        for line in ret['stderr']
    )
