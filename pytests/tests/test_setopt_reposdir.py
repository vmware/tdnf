#
# Copyright (C) 2021 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#
#   Author: Oliver Kurth <okurth@vmware.com>

import os
import shutil
import pytest
import errno

REPODIR='/root/yum.repos.d'
REPOFILENAME='reposync.repo'
REPONAME="reposdir-test"

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
    assert(REPONAME in "\n".join(ret['stdout']))

