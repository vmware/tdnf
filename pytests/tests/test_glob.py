#!/usr/bin/env python3

#
# Copyright (C) 2019-2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import pytest


@pytest.fixture(scope="module", autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    pass


def test_glob_install(utils):
    cmd = "tdnf remove -y tdnf*multi*".split()
    ret = utils.run(cmd)
    assert ret["retval"] == 0
    cmd = "rpm -qa 'tdnf*multi*'".split()
    ret = utils.run(cmd)
    assert ret["retval"] == 0

    cmd = "tdnf install -y tdnf*multi*".split()
    ret = utils.run(cmd)
    assert ret["retval"] == 0
    cmd = "rpm -q tdnf-multi tdnf-test-multiversion".split()
    ret = utils.run(cmd)
    assert ret["retval"] == 0


def test_glob_uninstall(utils):
    cmd = "tdnf remove -y tdnf*multi*".split()
    ret = utils.run(cmd)
    assert ret["retval"] == 0


def test_glob_uninstall_with_all_repos_disabled(utils):
    cmd = "tdnf install -y tdnf*multi*".split()
    ret = utils.run(cmd)
    assert ret["retval"] == 0

    cmd = "rpm -q tdnf-multi tdnf-test-multiversion".split()
    ret = utils.run(cmd)
    assert ret["retval"] == 0

    cmd = "tdnf remove -y tdnf*multi* --disablerepo=*".split()
    ret = utils.run(cmd)
    assert ret["retval"] == 0
    cmd = "rpm -q tdnf-multi tdnf-test-multiversion".split()
    ret = utils.run(cmd)
    assert ret["retval"]
