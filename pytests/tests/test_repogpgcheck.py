#
# Copyright (C) 2020-2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import os
import pytest

PLUGIN_NAME = 'tdnfrepogpgcheck'


@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    utils.edit_config({'plugins': '0',
                       'pluginconfpath': None,
                       'pluginpath': None})
    plugin_conf_path = os.path.join(utils.config['repo_path'], 'pluginconf.d')
    plugin_conf = os.path.join(plugin_conf_path, PLUGIN_NAME + '.conf')
    os.remove(plugin_conf)
    utils.edit_config({'repo_gpgcheck': None}, repo='photon-test')


def enable_plugins(utils):
    # write a plugin config file
    plugin_conf_path = os.path.join(utils.config['repo_path'], 'pluginconf.d')
    plugin_path = utils.config['plugin_path']
    utils.makedirs(plugin_conf_path)

    utils.edit_config({'plugins': '1',
                       'pluginconfpath': plugin_conf_path,
                       'pluginpath': plugin_path})

    plugin_conf = os.path.join(plugin_conf_path, PLUGIN_NAME + '.conf')
    with open(plugin_conf, 'w') as plugin_conf_file:
        plugin_conf_file.write('[main]\nenabled=1\n')


def set_plugin_flag_in_conf(utils, flag):
    utils.edit_config({'plugins': flag})


def set_plugin_config_enabled_flag(utils, flag):
    plugin_conf = os.path.join(utils.config['repo_path'], 'pluginconf.d', PLUGIN_NAME + '.conf')
    utils.edit_config({'enabled': flag}, filename=plugin_conf, section='main')


def set_repo_flag_repo_gpgcheck(utils, flag):
    utils.edit_config({'repo_gpgcheck': flag, 'skip_if_unavailable': 'False'}, repo='photon-test')


# make sure libtdnfrepogpgcheck.so is loaded without issues
def test_tdnfrepogpgcheck_plugin_load(utils):
    enable_plugins(utils)
    ret = utils.run(['tdnf', 'repolist'])
    # we should load the plugin
    assert ret['stdout'][0].startswith('Loaded plugin: tdnfrepogpgcheck')  # nosec
    # we should also pass the repolist command
    assert ret['retval'] == 0  # nosec


# make sure libtdnfrepogpgcheck.so is used to validate repo signature
def test_tdnfrepogpgcheck_plugin_validatesignature(utils):
    set_repo_flag_repo_gpgcheck(utils, "1")
    ret = utils.run(['tdnf', 'repolist', '--refresh'])
    # we should load the plugin
    assert ret['stdout'][0].startswith('Loaded plugin: tdnfrepogpgcheck')  # nosec
    assert ret['retval'] == 0
