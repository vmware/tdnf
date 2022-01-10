#
# Copyright (C) 2020-2022 VMware, Inc. All Rights Reserved.
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
    utils.run(['sed', '-i', '/plugins/d', tdnf_conf])
    utils.run(['sed', '-i', '/pluginconfpath/d', tdnf_conf])


def enable_plugins(utils):
    # write a plugin config file
    tdnf_conf = os.path.join(utils.config['repo_path'], 'tdnf.conf')
    utils.run(['sed', '-i', '2iplugins=1', tdnf_conf])
    plugin_conf_path = os.path.join(utils.config['repo_path'], 'pluginconf.d')
    utils.run(['mkdir', '-p', plugin_conf_path])
    utils.run(['sed', '-i', '2ipluginconfpath=' + plugin_conf_path, tdnf_conf])
    plugin_conf = os.path.join(plugin_conf_path, 'test_plugin.conf')
    with open(plugin_conf, 'w') as plugin_conf_file:
        plugin_conf_file.write('[main]\nenabled=1\n')


def set_plugin_flag_in_conf(utils, flag):
    tdnf_conf = os.path.join(utils.config['repo_path'], 'tdnf.conf')
    utils.run(['sed', '-i', '/plugins/d', tdnf_conf])
    utils.run(['sed', '-i', '2iplugins=' + flag, tdnf_conf])


def set_plugin_config_enabled_flag(utils, flag):
    test_plugin_conf = os.path.join(utils.config['repo_path'], 'pluginconf.d/test_plugin.conf')
    utils.run(['sed', '-i', '/enabled/d', test_plugin_conf])
    utils.run(['sed', '-i', '2ienabled=' + flag, test_plugin_conf])


def test_plugin_conf(utils):
    enable_plugins(utils)
    ret = utils.run(['tdnf', 'repolist'])
    # should attempt a plugin load
    assert(ret['stderr'][0].startswith('Error loading plugin'))  # nosec
    # plugin load failure should not affect command result
    assert(ret['retval'] == 0)  # nosec


# even when config file has plugins=1, --noplugins
# in command line should deactivate all plugins
def test_plugin_conf_override_no_plugins(utils):
    set_plugin_flag_in_conf(utils, '1')
    ret = utils.run(['tdnf', 'repolist', '--noplugins'])
    # expect no standard error lines - this corresponds to plugin load
    assert(len(ret['stderr']) == 0)  # nosec
    # command should pass
    assert(ret['retval'] == 0)  # nosec


# if plugins=0 in config, no plugins should be loaded.
def test_plugin_conf_disable_plugins_in_conf(utils):
    set_plugin_flag_in_conf(utils, '0')
    ret = utils.run(['tdnf', 'repolist'])
    # expect no standard error lines
    assert(len(ret['stderr']) == 0)  # nosec
    # command should pass
    assert(ret['retval'] == 0)  # nosec


# if all plugins are deactivated using wildcard,
# no plugin load should be attempted even if config is set
def test_plugin_command_line_disable_with_glob(utils):
    set_plugin_flag_in_conf(utils, '1')
    ret = utils.run(['tdnf', 'repolist', '--disableplugin=*'])
    # expect no standard error lines - this corresponds to plugin load
    assert(len(ret['stderr']) == 0)  # nosec
    # command should pass
    assert(ret['retval'] == 0)  # nosec


# enable all plugins by command line glob
# overrides plugins setting in config
def test_plugin_command_line_enable_with_glob(utils):
    set_plugin_flag_in_conf(utils, '1')
    ret = utils.run(['tdnf', 'repolist', '--enableplugin=*'])
    # stderr should have an attempted load
    assert(ret['stderr'][0].startswith('Error loading plugin'))  # nosec
    # command should pass
    assert(ret['retval'] == 0)  # nosec


# enable a specific plugin
# overrides plugins setting in config
def test_plugin_command_line_enable_single(utils):
    set_plugin_flag_in_conf(utils, '1')
    ret = utils.run(['tdnf', 'repolist', '--disableplugin=*', '--enableplugin=test_plugin'])
    # stderr should have an attempted load
    assert(ret['stderr'][0].startswith('Error loading plugin'))  # nosec
    # command should pass
    assert(ret['retval'] == 0)  # nosec


# deactivate individual plugin via plugin config
# ensure it is not loaded
def test_plugin_disable_via_plugin_config(utils):
    set_plugin_flag_in_conf(utils, '1')
    set_plugin_config_enabled_flag(utils, '0')
    ret = utils.run(['tdnf', 'repolist'])
    # stderr should not have an attempted load
    assert(len(ret['stderr']) == 0)  # nosec
    # command should pass
    assert(ret['retval'] == 0)  # nosec


# deactivate individual plugin via plugin config
# enable via command line
def test_plugin_enable_disabled_plugin_via_cmdline_override(utils):
    set_plugin_flag_in_conf(utils, '1')
    set_plugin_config_enabled_flag(utils, '0')
    ret = utils.run(['tdnf', 'repolist', '--enableplugin=test_plugin'])
    # stderr should have an attempted load
    assert(ret['stderr'][0].startswith('Error loading plugin'))  # nosec
    # command should pass
    assert(ret['retval'] == 0)  # nosec
