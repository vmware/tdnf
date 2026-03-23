#
# Copyright (C) 2021-2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import os
import glob
import shutil
import pytest
import fnmatch
import tempfile

INSTALLROOT = '/root/installroot'
REPOFILENAME = 'photon-test.repo'
RELEASE_VER = '5.0'

REPODIR = '/root/yum.repos.d'
REPONAME = 'reposdir-test'


@pytest.fixture(scope='function', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    if os.path.isdir(INSTALLROOT):
        shutil.rmtree(INSTALLROOT)
    if os.path.isdir(REPODIR):
        shutil.rmtree(REPODIR)


def install_root(utils, no_reposd=False):
    os.makedirs(INSTALLROOT, exist_ok=True)
    os.makedirs(os.path.join(INSTALLROOT, 'etc/tdnf'), exist_ok=True)
    conffile = os.path.join(utils.config['repo_path'], 'tdnf.conf')

    # remove special settings for repodir and cachedir
    with open(conffile, 'r') as fin:
        with open(os.path.join(INSTALLROOT, 'etc/tdnf', 'tdnf.conf'), 'w') as fout:
            for line in fin:
                if not line.startswith('repodir') and \
                   not line.startswith('cachedir'):
                    fout.write(line)

    if not no_reposd:
        os.makedirs(os.path.join(INSTALLROOT, 'etc/yum.repos.d'), exist_ok=True)
        repofile = os.path.join(utils.config['repo_path'], "yum.repos.d", REPOFILENAME)
        shutil.copyfile(repofile, os.path.join(INSTALLROOT, 'etc/yum.repos.d', REPOFILENAME))
    os.makedirs(os.path.join(INSTALLROOT, 'var/cache/tdnf'), exist_ok=True)
    utils.run(['rpm', '--root', INSTALLROOT, '--initdb'])


# local version of check_package with install root
def check_package(utils, package, installroot=INSTALLROOT, version=None):
    """ Check if a package exists """
    ret = utils.run(['tdnf',
                     '--installroot', installroot,
                     '--releasever=4.0',
                     'list', package])
    for line in ret['stdout']:
        if package in line and '@System' in line:
            if version is None or version in line:
                return True
    return False


def erase_package(utils, pkgname, installroot=INSTALLROOT, pkgversion=None):
    if pkgversion:
        pkg = pkgname + '-' + pkgversion
    else:
        pkg = pkgname
    utils.run(['tdnf',
               '--installroot', installroot,
               '--releasever=4.0',
               'erase', '-y', pkg])
    assert not check_package(utils, pkgname)


def find_cache_dir(reponame):
    cache_dir = os.path.join(INSTALLROOT, 'var/cache/tdnf')
    for f in os.listdir(cache_dir):
        if fnmatch.fnmatch(f, '{}-*'.format(reponame)):
            return os.path.join(cache_dir, f)
    return None


def test_install(utils):
    install_root(utils)
    pkgname = utils.config["mulversion_pkgname"]
    erase_package(utils, pkgname)

    ret = utils.run(['tdnf', 'install',
                     '-y', '--nogpgcheck',
                     '--installroot', INSTALLROOT,
                     '--releasever=4.0', pkgname], noconfig=True)
    assert ret['retval'] == 0
    assert check_package(utils, pkgname)

    shutil.rmtree(INSTALLROOT)


def test_makecache(utils):
    install_root(utils)
    ret = utils.run(['tdnf', 'makecache',
                     '-y', '--nogpgcheck',
                     '--installroot', INSTALLROOT,
                     '--releasever=4.0'], noconfig=True)
    assert ret['retval'] == 0
    assert find_cache_dir('photon-test') is not None

    shutil.rmtree(INSTALLROOT)


# --setopt=reposdir overrides any dir in install root
def test_setopt_reposdir_with_installroot(utils):
    install_root(utils)
    os.makedirs(REPODIR, exist_ok=True)
    utils.create_repoconf(os.path.join(REPODIR, REPOFILENAME),
                          "http://foo.bar.com/packages",
                          REPONAME)
    ret = utils.run(['tdnf',
                     '--installroot', INSTALLROOT,
                     '--releasever=4.0',
                     '--setopt=reposdir={}'.format(REPODIR),
                     'repolist'])
    assert REPONAME in "\n".join(ret['stdout'])

    shutil.rmtree(INSTALLROOT)


def test_installroot_local_rpms_disablerepo_after_download(utils):

    workdir = tempfile.mkdtemp(prefix='test-tdnf-')
    try:
        installroot = os.path.join(workdir, 'installroot')
        rpm_repo = os.path.join(workdir, 'rpm-repo')
        os.makedirs(rpm_repo, exist_ok=True)
        os.makedirs(installroot, exist_ok=True)

        repo_url = "http://localhost:8080/photon-test"

        ret = utils.run(
            ['tdnf', 'install', '-y', 'tdnf-test3', 'tdnf-test4',
             '--refresh', '--downloadonly',
             '--releasever', RELEASE_VER,
             '--installroot', installroot,
             '--downloaddir', rpm_repo,
             '--nogpgcheck',
             '--disablerepo=*', '--enablerepo=pkgs',
             f'--repofrompath=pkgs,{repo_url}'],
            cwd=workdir
        )
        assert ret['retval'] == 0, 'downloadonly failed: {}'.format(ret.get('stderr', []))

        rpms = glob.glob(os.path.join(rpm_repo, '*.rpm'))
        assert len(rpms) >= 1, 'no RPMs downloaded into rpm-repo'

        ret = utils.run(
            ['tdnf', 'install', '-y', '--refresh', '--releasever', RELEASE_VER,
             '--installroot', installroot, '--disablerepo=*', '--nogpgcheck'] + rpms,
            cwd=workdir
        )
        assert ret['retval'] == 0, (
            'install from local RPMs into installroot with --disablerepo=* failed: {}'
        ).format(ret.get('stderr', []))
    finally:
        shutil.rmtree(workdir, ignore_errors=True)
