#
# Copyright (C) 2021-2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import os
import shutil
import pytest

REPODIR = '/root/yum.repos.d'
REPOFILENAME = 'setopt.repo'
REPONAME = "reposdir-test"


@pytest.fixture(scope='function', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    if os.path.isdir(REPODIR):
        shutil.rmtree(REPODIR)


def test_setopt_reposdir(utils):
    utils.makedirs(REPODIR)
    utils.create_repoconf(os.path.join(REPODIR, REPOFILENAME),
                          "http://foo.bar.com/packages",
                          REPONAME)
    ret = utils.run(['tdnf', '--setopt=reposdir={}'.format(REPODIR), 'repolist'])
    assert ret['retval'] == 0
    assert REPONAME in "\n".join(ret['stdout'])
