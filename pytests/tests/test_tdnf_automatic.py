#
# Copyright (C) 2020-2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import os
import glob
import pytest
import shutil
import socket
import configparser

tdnf_conf = None
automatic_cmd = None
automatic_conf = None

tmp_auto_conf = '/tmp/auto.conf'
emit_file = '/tmp/tdnf-auto.txt'
repo_name = 'photon-test'

ini = configparser.ConfigParser()


@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    global automatic_conf, tdnf_conf, automatic_cmd
    automatic_conf = utils.config['automatic_conf']
    tdnf_conf = os.path.join(utils.config['repo_path'], 'tdnf.conf')
    automatic_cmd = utils.config['automatic_script']
    repo_file = get_repo_file_path()
    shutil.copy(repo_file, repo_file + '.bak')

    # make sure to use the built tdnf binary, not the installed one by
    # prefixing the build/bin directory
    os.environ['PATH'] = utils.config['bin_dir'] + ':' + os.environ['PATH']

    cleanup_env(utils)
    yield
    teardown_test(utils)


def teardown_test(utils):
    repo_file = get_repo_file_path()
    os.rename(repo_file + '.bak', repo_file)
    cleanup_env(utils)


def cleanup_env(utils):
    pkgname = utils.config["mulversion_pkgname"]
    utils.erase_package(pkgname)
    if os.path.exists(tmp_auto_conf):
        os.unlink(tmp_auto_conf)
    files = glob.glob(emit_file + '*')
    for f in files:
        os.unlink(f)


def get_repo_file_path():
    ini_load(tdnf_conf)
    return os.path.join(ini['main']['repodir'], repo_name + '.repo')


def ini_load(ini_file):
    ini.clear()
    ini.read(ini_file)


def ini_set(sec, key, val):
    ini.set(sec, key, val)


def ini_store(fname):
    with open(fname, 'w') as f:
        ini.write(f)


def set_base_conf_and_store(fname):
    ini_set('base', 'tdnf_conf', tdnf_conf)
    ini_store(fname)


def ini_load_set_store(fname, sec, key, val):
    ini_load(fname)
    ini_set(sec, key, val)
    ini_store(fname)


def rm_all_spaces_in_file(fname):
    with open(fname, 'r') as f:
        lines = f.readlines()
    # remove spaces
    lines = [line.replace(' ', '') for line in lines]

    # finally, write lines in the file
    with open(fname, 'w') as f:
        f.writelines(lines)


def run_test_cmd(utils, cmd):
    return utils._run(cmd)


def prepare_and_run_test_cmd(utils, opt, retval, retstr=''):
    tmp = [automatic_cmd]
    if '-c' not in opt and '--config' not in opt:
        tmp += ['-c', tmp_auto_conf]
    ret = run_test_cmd(utils, tmp + opt)
    assert ret['retval'] == retval

    if not retstr:
        return 0

    found = False
    output_str = ret['stderr'] if retval else ret['stdout']

    for i in output_str:
        if retstr in i:
            found = True
            break

    assert found


def test_tdnf_automatic_show_help(utils):
    prepare_and_run_test_cmd(utils, ['-h'], 0, 'tdnf-automatic help:')


def test_tdnf_automatic_show_version(utils):
    prepare_and_run_test_cmd(utils, ['-v'], 0, 'tdnf-automatic - version:')


def test_tdnf_automatic_invalid_arg(utils):
    prepare_and_run_test_cmd(utils, ['--invalid'], 22, 'tdnf-automatic help:')


def test_tdnf_automatic_invalid_cfg_opt1(utils):
    ini_load(automatic_conf)
    ini_set('commands', 'show_updates', 'badbool')
    set_base_conf_and_store(tmp_auto_conf)
    prepare_and_run_test_cmd(utils, [], 22, 'Invalid input')


def test_tdnf_automatic_invalid_cfg_opt2(utils):
    ini_load(automatic_conf)
    ini_set('commands', 'upgrade_type', 'badval')
    ini_load(automatic_conf)
    prepare_and_run_test_cmd(utils, [], 22, 'Invalid entry')


def test_tdnf_automatic_invalid_tdnf_conf(utils):
    ini_load(automatic_conf)
    ini_set('base', 'tdnf_conf', '/badpath/badfile.conf')
    ini_store(tmp_auto_conf)
    prepare_and_run_test_cmd(utils, [], 2, 'does not exist')


def test_tdnf_automatic_invalid_automatic_conf(utils):
    prepare_and_run_test_cmd(utils, ['-c', '/badpath/badfile.conf'], 2, 'does not exist')


def test_tdnf_automatic_rand_sleep(utils):
    ini_load(automatic_conf)
    ini_set('commands', 'random_sleep', '3')
    set_base_conf_and_store(tmp_auto_conf)
    prepare_and_run_test_cmd(utils, ['-t'], 0, 'Sleep for')


def test_tdnf_automatic_refresh_cache(utils):
    ini_load(automatic_conf)
    set_base_conf_and_store(tmp_auto_conf)
    prepare_and_run_test_cmd(utils, [], 0, 'RefreshCache success')


def test_tdnf_automatic_disable_repos_retry(utils):
    repo_file = get_repo_file_path()
    ini_load_set_store(repo_file, repo_name, 'enabled', '0')
    rm_all_spaces_in_file(repo_file)
    ini_load(automatic_conf)
    ini_set('commands', 'network_online_timeout', '3')
    set_base_conf_and_store(tmp_auto_conf)
    prepare_and_run_test_cmd(utils, [], 64, 'System is off-line')
    ini_load_set_store(repo_file, repo_name, 'enabled', '1')
    rm_all_spaces_in_file(repo_file)


def test_tdnf_automatic_invalid_baseurl(utils):
    repo_file = get_repo_file_path()
    ini_load(repo_file)
    original_baseurl = ini[repo_name]['baseurl']
    ini_load_set_store(repo_file, repo_name, 'baseurl', 'https:/invalidurl.test')
    rm_all_spaces_in_file(repo_file)
    ini_load(automatic_conf)
    ini_set('commands', 'network_online_timeout', '3')
    set_base_conf_and_store(tmp_auto_conf)
    prepare_and_run_test_cmd(utils, [], 64, 'System is off-line')
    ini_load_set_store(repo_file, repo_name, 'baseurl', original_baseurl)
    rm_all_spaces_in_file(repo_file)


def test_emit_to_stdio(utils):
    ini_load(automatic_conf)
    ini_set('emitter', 'emit_to_stdio', 'yes')
    set_base_conf_and_store(tmp_auto_conf)
    prepare_and_run_test_cmd(utils, [], 0, 'System upto date')


def test_emit_to_file(utils):
    ini_load(automatic_conf)
    ini_set('emitter', 'emit_to_stdio', 'no')
    ini_set('emitter', 'emit_to_file', emit_file)
    set_base_conf_and_store(tmp_auto_conf)
    prepare_and_run_test_cmd(utils, [], 0)
    with open(emit_file) as f:
        assert 'System upto date' in f.read()

    prepare_and_run_test_cmd(utils, [], 0)
    glob.glob(emit_file + '*.bak')


def test_tdnf_automatic_show_updates(utils):
    cleanup_env(utils)
    pkgname = utils.config["mulversion_pkgname"]
    pkgversion = utils.config["mulversion_lower"]
    utils.run(['tdnf', 'install', '-y', pkgname + '-' + pkgversion])

    ini_load(automatic_conf)
    ini_set('emitter', 'emit_to_stdio', 'yes')
    ini_set('emitter', 'emit_to_file', emit_file)
    ini_set('commands', 'show_updates', 'yes')
    ini_set('commands', 'apply_updates', 'no')
    set_base_conf_and_store(tmp_auto_conf)
    prepare_and_run_test_cmd(utils, [], 0, 'The following updates are available on')
    with open(emit_file) as f:
        assert pkgname in f.read()


def test_tdnf_automatic_apply_updates(utils):
    cleanup_env(utils)
    pkgname = utils.config["mulversion_pkgname"]
    pkgversion = utils.config["mulversion_lower"]
    utils.run(['tdnf', 'install', '-y', pkgname + '-' + pkgversion])

    ini_load(automatic_conf)
    ini_set('emitter', 'emit_to_stdio', 'yes')
    ini_set('emitter', 'emit_to_file', emit_file)
    ini_set('commands', 'show_updates', 'no')
    ini_set('commands', 'apply_updates', 'yes')
    set_base_conf_and_store(tmp_auto_conf)
    prepare_and_run_test_cmd(utils, [], 0, 'The following updates are applied on')
    with open(emit_file) as f:
        assert pkgname in f.read()


def test_tdnf_automatic_same_show_apply_updates(utils):
    cleanup_env(utils)
    pkgname = utils.config["mulversion_pkgname"]
    pkgversion = utils.config["mulversion_lower"]
    utils.run(['tdnf', 'install', '-y', pkgname + '-' + pkgversion])

    ini_load(automatic_conf)
    ini_set('emitter', 'emit_to_stdio', 'yes')
    ini_set('commands', 'show_updates', 'yes')
    ini_set('commands', 'apply_updates', 'yes')
    set_base_conf_and_store(tmp_auto_conf)
    prepare_and_run_test_cmd(utils, [], 0, 'The following updates are available on')
    assert not os.path.isfile(emit_file)
    ini_set('commands', 'show_updates', 'no')
    ini_set('commands', 'apply_updates', 'no')
    set_base_conf_and_store(tmp_auto_conf)
    prepare_and_run_test_cmd(utils, [], 0, 'The following updates are available on')
    assert not os.path.isfile(emit_file)


def test_tdnf_automatic_hostname(utils):
    cleanup_env(utils)
    pkgname = utils.config["mulversion_pkgname"]
    pkgversion = utils.config["mulversion_lower"]
    utils.run(['tdnf', 'install', '-y', pkgname + '-' + pkgversion])
    for i in [socket.gethostname(), 'test-hostname']:
        ini_set('emitter', 'emit_to_stdio', 'yes')
        ini_set('emitter', 'emit_to_file', emit_file)
        ini_set('commands', 'show_updates', 'yes')
        ini_set('commands', 'apply_updates', 'no')
        ini_set('emitter', 'system_name', i)
        set_base_conf_and_store(tmp_auto_conf)
        prepare_and_run_test_cmd(utils, [], 0, 'The following updates are available on - ' + i)
        with open(emit_file) as f:
            assert i in f.read()
