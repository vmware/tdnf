#
# Copyright (C) 2020 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import os
import pytest

@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)

def teardown_test(utils):
    tdnf_conf = os.path.join(utils.config['repo_path'], 'tdnf.conf')
    utils.run([ 'sed', '-i', '/plugins/d', tdnf_conf])
    utils.run([ 'sed', '-i', '/pluginconfpath/d', tdnf_conf])
    plugin_conf_path = os.path.join(utils.config['repo_path'], 'pluginconf.d')
    plugin_conf = os.path.join(plugin_conf_path, 'tdnfrepogpgcheck.conf')
    os.remove(plugin_conf)
    tdnf_repo = os.path.join(utils.tdnf_config.get('main', 'repodir'), 'photon-test.repo')
    utils.run([ 'sed', '-i', '/repo_gpgcheck/d', tdnf_repo])

def enable_plugins(utils):
    #write a plugin config file
    tdnf_conf = os.path.join(utils.config['repo_path'], 'tdnf.conf')
    utils.run([ 'sed', '-i', '2iplugins=1', tdnf_conf])
    plugin_conf_path = os.path.join(utils.config['repo_path'], 'pluginconf.d')
    utils.run([ 'mkdir', '-p', plugin_conf_path ])
    utils.run([ 'sed', '-i', '2ipluginconfpath=' + plugin_conf_path, tdnf_conf])
    plugin_path = utils.config['plugin_path']
    utils.run([ 'sed', '-i', '2ipluginpath=' + plugin_path, tdnf_conf])
    plugin_conf = os.path.join(plugin_conf_path, 'tdnfrepogpgcheck.conf')
    with open(plugin_conf, 'w') as plugin_conf_file:
        plugin_conf_file.write('[main]\nenabled=1\n')

def set_plugin_flag_in_conf(utils, flag):
    tdnf_conf = os.path.join(utils.config['repo_path'], 'tdnf.conf')
    utils.run([ 'sed', '-i', '/plugins/d', tdnf_conf])
    utils.run([ 'sed', '-i', '2iplugins=' + flag, tdnf_conf])

def set_plugin_config_enabled_flag(utils, flag):
    tdnfrepogpgcheck_plugin_conf = os.path.join(utils.config['repo_path'], 'pluginconf.d/tdnfrepogpgcheck.conf')
    utils.run([ 'sed', '-i', '/enabled/d', tdnfrepogpgcheck_plugin_conf])
    utils.run([ 'sed', '-i', '2ienabled=' + flag, tdnfrepogpgcheck_plugin_conf])

def set_repo_flag_repo_gpgcheck(utils, flag):
    tdnf_repo = os.path.join(utils.tdnf_config.get('main', 'repodir'), 'photon-test.repo')
    utils.run([ 'sed', '-i', '2irepo_gpgcheck=' + flag, tdnf_repo])
    utils.run([ 'sed', '-i', '2iskip_if_unavailable=False', tdnf_repo])

#make sure libtdnfrepogpgcheck.so is loaded without issues
def test_tdnfrepogpgcheck_plugin_load(utils):
    enable_plugins(utils)
    ret = utils.run([ 'tdnf', 'repolist' ])
    #we should load the plugin
    assert(ret['stdout'][0].startswith('Loaded plugin: tdnfrepogpgcheck')) #nosec
    #we should also pass the repolist command
    assert(ret['retval'] == 0) #nosec

#make sure libtdnfrepogpgcheck.so is used to validate repo signature
def test_tdnfrepogpgcheck_plugin_validatesignature(utils):
    set_repo_flag_repo_gpgcheck(utils, "1")
    ret = utils.run([ 'tdnf', 'repolist', '--refresh'])
    #we should load the plugin
    assert(ret['stdout'][0].startswith('Loaded plugin: tdnfrepogpgcheck')) #nosec
    assert(ret['retval'] == 0)
