#
# Copyright (C) 2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import pytest

pkg0 = "tdnf-conflict-file0"
pkg1 = "tdnf-conflict-file1"


@pytest.fixture(scope='function', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    utils.run(['tdnf', 'erase', '-y', pkg0])
    utils.run(['tdnf', 'erase', '-y', pkg1])


def test_install_conflict_file(utils):
    ret = utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkg0])
    print(ret)
    assert utils.check_package(pkg0)

    ret = utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkg1])
    print(ret)
    assert ret['retval'] == 1525
    assert not utils.check_package(pkg1)


def test_install_conflict_file_atonce(utils):
    ret = utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkg0, pkg1])
    print(ret)
    assert ret['retval'] == 1525
    assert not utils.check_package(pkg0)
    assert not utils.check_package(pkg1)
