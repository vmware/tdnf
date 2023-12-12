#
# Copyright (C) 2023 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import os
import shutil
import pytest


INSTALLROOT = '/root/installroot'
REPOFILENAME = 'photon-test.repo'


@pytest.fixture(scope='function', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    if os.path.isdir(INSTALLROOT):
        shutil.rmtree(INSTALLROOT)


@pytest.mark.parametrize("dbpath", ["/usr/lib/rpm", "/usr/lib/sysimage/rpm/"])
def test_install(utils, dbpath):
    pkgname = utils.config["mulversion_pkgname"]
    ret = utils.run(['tdnf', 'install',
                     '-y', '--nogpgcheck',
                     '--installroot', INSTALLROOT,
                     '--releasever=5.0',
                     '--rpmdefine', f"_dbpath {dbpath}",
                     pkgname])
    assert ret['retval'] == 0
    assert os.path.isdir(os.path.join(INSTALLROOT, dbpath.lstrip("/")))
    assert os.path.isfile(os.path.join(INSTALLROOT, dbpath.lstrip("/"), "rpmdb.sqlite"))
