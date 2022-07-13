#
# Copyright (C) 2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import os
import platform
import pytest
from subprocess import Popen, PIPE

pkgname = None
test_usr = 'test_usr'


def run_bash_cmd(cmd, rc, bash_cmd=False):
    print(' '.join(cmd))
    proc = Popen(cmd, stdout=PIPE, stderr=PIPE, stdin=PIPE)
    proc.wait()

    if rc == 'ignore':
        return

    out, err = proc.communicate()
    out = out.decode().strip().split('\n')
    err = err.decode().strip().split('\n')
    print('stdout:', out, '\nstderr:', err)

    if bash_cmd:
        assert rc == (not not proc.returncode)
        return

    if not rc:
        assert 'You have to be root' not in err
    else:
        assert 'You have to be root' in err

    assert rc == (not not proc.returncode)
    return [out, err]


def run_tdnf_cmd(utils, cmd, rc):
    cmd.insert(0, 'tdnf')
    utils._decorate_tdnf_cmd_for_test(cmd)
    cmd = ' '.join(cmd)
    cmd = ['sudo', '-u', test_usr, 'bash', '-c', cmd]
    return run_bash_cmd(cmd, rc, bash_cmd=True)


@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    run_bash_cmd(['userdel', '-f', test_usr], 'ignore')
    run_bash_cmd(['useradd', test_usr], 0, bash_cmd=True)

    global pkgname
    pkgname = utils.config['sglversion_pkgname']

    if not utils.config.get('installed', False):
        build_dir = utils.config['build_dir']
        test_dir = build_dir + '/pytests'
        bin_dir = utils.config['bin_dir']
        tdnf_bin = os.path.join(bin_dir, 'tdnf')
        lib_dir = build_dir + '/lib'

        dirs = ''
        path = []
        for ele in build_dir.split(os.sep):
            if not ele:
                continue
            dirs += '/' + ele
            path.append(dirs)

        path += [bin_dir, test_dir, lib_dir, tdnf_bin]
        run_bash_cmd(['chmod', '755'] + path, 0, bash_cmd=True)
        os.system('chmod 644 ' + build_dir + '/lib/*')

    utils.run(['tdnf', 'makecache'])

    yield
    teardown_test(utils)


def teardown_test(utils):
    utils.run(['tdnf', 'erase', '-y', pkgname])
    run_bash_cmd(['chmod', '700', '/root'], 0, bash_cmd=True)
    run_bash_cmd(['userdel', '-f', test_usr], 'ignore')


def test_check(utils):
    run_tdnf_cmd(utils, ['check'], 'ignore')


def test_check_update(utils):
    run_tdnf_cmd(utils, ['check-update'], 0)


def test_count(utils):
    run_tdnf_cmd(utils, ['count'], 0)


def test_help(utils):
    run_tdnf_cmd(utils, ['help'], 0)


def test_info(utils):
    utils.run(['tdnf', 'install', '-y', pkgname])
    run_tdnf_cmd(utils, ['info', pkgname], 0)


def test_list(utils):
    run_tdnf_cmd(utils, ['list', 'installed'], 0)


def test_provides(utils):
    run_tdnf_cmd(utils, ['provides', pkgname], 0)


def test_whatprovides(utils):
    run_tdnf_cmd(utils, ['whatprovides', pkgname], 0)


def test_repolist(utils):
    run_tdnf_cmd(utils, ['repolist', 'all'], 0)


def test_reposync(utils):
    dwld_path = '/tmp/tdnf-tmp'
    run_bash_cmd(['chmod', '777', '/tmp'], 0, bash_cmd=True)
    run_bash_cmd(['mkdir', '-p', '-m', '777', dwld_path], 0, bash_cmd=True)
    run_tdnf_cmd(utils, ['reposync', '--download-path=' + dwld_path], 0)

    ARCH = platform.machine()
    rpm_dir = dwld_path + '/photon-test/RPMS/' + ARCH
    run_tdnf_cmd(utils, ['check-local', rpm_dir], 'ignore')

    run_bash_cmd(['rm', '-rf', dwld_path], 0, bash_cmd=True)


def test_repoquery(utils):
    run_tdnf_cmd(utils, ['repoquery', pkgname], 0)


def test_search(utils):
    run_tdnf_cmd(utils, ['search', pkgname], 0)


def test_update_info(utils):
    run_tdnf_cmd(utils, ['updateinfo'], 0)


def test_install(utils):
    run_tdnf_cmd(utils, ['install', '-y', pkgname], 1)


def test_erase(utils):
    run_tdnf_cmd(utils, ['erase', '-y', pkgname], 1)


def test_update(utils):
    run_tdnf_cmd(utils, ['update', '-y', pkgname], 1)
