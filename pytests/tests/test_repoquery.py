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
import errno
import pytest

BASE_PKG = 'tdnf-repoquery-base'

@pytest.fixture(scope='function', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)

def teardown_test(utils):
    pass

# repoquery should list all packages that depend on BASE_PKG
# (one of 'enhances', 'recommends', 'requires', 'suggests', 'supplements')
def test_whatdepends(utils):
    ret = utils.run(['tdnf',
                     'repoquery',
                     '--whatdepends',
                     BASE_PKG])
    assert(ret['retval'] == 0)

    for d in ['enhances', 'recommends', 'requires', 'suggests', 'supplements']:
        assert('tdnf-repoquery-{}'.format(d) in '\n'.join(ret['stdout']))

# repoquery should list the package that has a specific
# relation to BASE_PKG
def test_what_alldeps(utils):
    dep_types = ['conflicts', 'enhances', 'obsoletes', 'provides',
                 'recommends', 'requires', 'suggests', 'supplements']

    for dep in dep_types:
        ret = utils.run(['tdnf',
                         'repoquery',
                         '--what{}'.format(dep),
                         BASE_PKG
                        ])
        assert(ret['retval'] == 0)
        assert('tdnf-repoquery-{}'.format(dep) in '\n'.join(ret['stdout']))

# repoquery should list the package that requires BASE_PKG
# or some other package that may or may not exist. Tests multiple
# args to --whatrequires separated by a comma
def test_what_2(utils):

    ret = utils.run(['tdnf',
                     'repoquery',
                     '--whatrequires',
                     "{},{}".format('doesnotexist', BASE_PKG)
                    ])
    assert(ret['retval'] == 0)
    assert('tdnf-repoquery-requires' in '\n'.join(ret['stdout']))

