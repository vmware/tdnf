#
# Copyright (C) 2024 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import os
import shutil
import pytest


@pytest.fixture(scope='function', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    utils.run(['tdnf', 'lock', 'clear'])
    ldir = locks_dir(utils)
    if os.path.isdir(ldir):
        shutil.rmtree(ldir)
    pkgname = utils.config['sglversion_pkgname']
    utils.erase_package(pkgname)


def locks_dir(utils):
    return os.path.join(utils.config['repo_path'], 'locks.d')


def lock_file(utils):
    return os.path.join(locks_dir(utils), 'versionlock.conf')


def locked_packages(utils):
    lf = lock_file(utils)
    if not os.path.exists(lf):
        return []
    with open(lf) as f:
        return [line.strip() for line in f
                if line.strip() and not line.startswith('#')]


def test_lock_list_empty(utils):
    ret = utils.run(['tdnf', 'lock', 'list'])
    assert ret['retval'] == 0


def test_lock_add(utils):
    pkgname = utils.config['sglversion_pkgname']
    ret = utils.run(['tdnf', 'lock', 'add', pkgname])
    assert ret['retval'] == 0
    assert pkgname in locked_packages(utils)


def test_lock_add_duplicate(utils):
    pkgname = utils.config['sglversion_pkgname']
    utils.run(['tdnf', 'lock', 'add', pkgname])
    ret = utils.run(['tdnf', 'lock', 'add', pkgname])
    assert ret['retval'] == 0
    assert locked_packages(utils).count(pkgname) == 1


def test_lock_list_after_add(utils):
    pkgname = utils.config['sglversion_pkgname']
    utils.run(['tdnf', 'lock', 'add', pkgname])
    ret = utils.run(['tdnf', 'lock', 'list'])
    assert ret['retval'] == 0
    assert any(pkgname in line for line in ret['stdout'])


def test_lock_delete(utils):
    pkgname = utils.config['sglversion_pkgname']
    utils.run(['tdnf', 'lock', 'add', pkgname])
    assert pkgname in locked_packages(utils)
    ret = utils.run(['tdnf', 'lock', 'delete', pkgname])
    assert ret['retval'] == 0
    assert pkgname not in locked_packages(utils)


def test_lock_clear(utils):
    pkg1 = utils.config['sglversion_pkgname']
    pkg2 = utils.config['mulversion_pkgname']
    utils.run(['tdnf', 'lock', 'add', pkg1, pkg2])
    assert len(locked_packages(utils)) == 2
    ret = utils.run(['tdnf', 'lock', 'clear'])
    assert ret['retval'] == 0
    assert locked_packages(utils) == []


def test_lock_prevents_erase(utils):
    pkgname = utils.config['sglversion_pkgname']
    utils.install_package(pkgname)
    utils.run(['tdnf', 'lock', 'add', pkgname])
    utils.run(['tdnf', '-y', 'remove', pkgname])
    assert utils.check_package(pkgname)


def test_mark_lock(utils):
    pkgname = utils.config['sglversion_pkgname']
    ret = utils.run(['tdnf', 'mark', 'lock', pkgname])
    assert ret['retval'] == 0
    assert pkgname in locked_packages(utils)


def test_mark_unlock(utils):
    pkgname = utils.config['sglversion_pkgname']
    utils.run(['tdnf', 'mark', 'lock', pkgname])
    assert pkgname in locked_packages(utils)
    ret = utils.run(['tdnf', 'mark', 'unlock', pkgname])
    assert ret['retval'] == 0
    assert pkgname not in locked_packages(utils)


def test_lock_add_multiple(utils):
    pkg1 = utils.config['sglversion_pkgname']
    pkg2 = utils.config['mulversion_pkgname']
    ret = utils.run(['tdnf', 'lock', 'add', pkg1, pkg2])
    assert ret['retval'] == 0
    pkgs = locked_packages(utils)
    assert pkg1 in pkgs
    assert pkg2 in pkgs


def test_lock_unknown_subcommand(utils):
    ret = utils.run(['tdnf', 'lock', 'bogus'])
    assert ret['retval'] != 0


def write_isolated_config(conf, tmp_path):
    conf.parent.mkdir(parents=True, exist_ok=True)
    repodir = tmp_path / 'empty-repos'
    repodir.mkdir(exist_ok=True)
    conf.write_text('[main]\nplugins=0\nrepodir={}\ncachedir={}\n'.format(
        repodir, tmp_path / 'cache'))


@pytest.mark.parametrize('action', [('lock', 'add'), ('mark', 'lock')])
def test_lock_relative_config(utils, tmp_path, action):
    conf = tmp_path / 'tdnf.conf'
    write_isolated_config(conf, tmp_path)
    args = ['tdnf', '-c', conf.name, '--releasever=5.0']
    pkgname = 'lock-config-path-test'
    ret = utils.run(args + list(action) + [pkgname], cwd=str(tmp_path))
    assert ret['retval'] == 0
    lockfile = tmp_path / 'locks.d' / 'versionlock.conf'
    assert lockfile.read_text().splitlines() == [pkgname]

    ret = utils.run(args + ['lock', 'list'], cwd=str(tmp_path))
    assert ret['retval'] == 0
    assert pkgname in ret['stdout']

    remove = ['mark', 'unlock'] if action[0] == 'mark' else ['lock', 'delete']
    ret = utils.run(args + remove + [pkgname], cwd=str(tmp_path))
    assert ret['retval'] == 0
    assert lockfile.read_text() == ''


@pytest.mark.parametrize('action', [('lock', 'add'), ('mark', 'lock')])
def test_lock_installroot_config(utils, tmp_path, action):
    root = tmp_path / 'root'
    conf = root / 'etc' / 'tdnf' / 'tdnf.conf'
    write_isolated_config(conf, tmp_path)
    args = ['tdnf', '--installroot', str(root), '--releasever=5.0']
    pkgname = 'lock-installroot-test'
    ret = utils.run(args + list(action) + [pkgname], noconfig=True)
    assert ret['retval'] == 0
    lockfile = conf.parent / 'locks.d' / 'versionlock.conf'
    assert lockfile.read_text().splitlines() == [pkgname]

    ret = utils.run(args + ['lock', 'list'], noconfig=True)
    assert ret['retval'] == 0
    assert pkgname in ret['stdout']

    ret = utils.run(args + ['lock', 'clear'], noconfig=True)
    assert ret['retval'] == 0
    assert lockfile.read_text() == ''


def test_lock_explicit_config_overrides_installroot(utils, tmp_path):
    root = tmp_path / 'root'
    root_conf = root / 'etc' / 'tdnf' / 'tdnf.conf'
    write_isolated_config(root_conf, tmp_path)
    root_locks = root_conf.parent / 'locks.d' / 'versionlock.conf'
    root_locks.parent.mkdir()
    root_locks.write_text('existing-root-lock\n')
    conf = tmp_path / 'tdnf.conf'
    write_isolated_config(conf, tmp_path)

    ret = utils.run(['tdnf', '-c', conf.name, '--installroot', str(root),
                     '--releasever=5.0', 'lock', 'add', 'explicit-config-lock'],
                    cwd=str(tmp_path))
    assert ret['retval'] == 0
    lockfile = tmp_path / 'locks.d' / 'versionlock.conf'
    assert lockfile.read_text().splitlines() == ['explicit-config-lock']
    assert root_locks.read_text() == 'existing-root-lock\n'
