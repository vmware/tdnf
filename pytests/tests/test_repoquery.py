#
# Copyright (C) 2021-2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import pytest

BASE_PKG = 'tdnf-repoquery-base'


@pytest.fixture(scope='function', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    utils.run(['tdnf', 'remove', '-y', BASE_PKG])


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


# packages should have specified relation to BASE_PKG
def test_alldeps(utils):
    dep_types = ['conflicts', 'enhances', 'obsoletes', 'provides',
                 'recommends', 'requires', 'suggests', 'supplements']

    for dep in dep_types:
        ret = utils.run(['tdnf',
                         'repoquery',
                         '--{}'.format(dep),
                         'tdnf-repoquery-{}'.format(dep)
                         ])
        assert(ret['retval'] == 0)
        assert(BASE_PKG in '\n'.join(ret['stdout']))


# all these packages depend on BASE_PKG
def test_depends(utils):
    for dep in ['enhances', 'recommends', 'requires', 'suggests', 'supplements']:
        ret = utils.run(['tdnf',
                         'repoquery',
                         '--depends',
                         'tdnf-repoquery-{}'.format(dep)])
        assert(ret['retval'] == 0)
        assert(BASE_PKG in '\n'.join(ret['stdout']))


# each package has a file with its name
def test_list(utils):
    dep_types = ['conflicts', 'enhances', 'obsoletes', 'provides',
                 'recommends', 'requires', 'suggests', 'supplements']

    for dep in dep_types:
        ret = utils.run(['tdnf',
                         'repoquery',
                         '--list',
                         'tdnf-repoquery-{}'.format(dep)
                         ])
        assert(ret['retval'] == 0)
        assert('/usr/lib/repoquery/tdnf-repoquery-{}'.format(dep) in '\n'.join(ret['stdout']))


# like test_list(), but the other way around
def test_file(utils):
    dep_types = ['conflicts', 'enhances', 'obsoletes', 'provides',
                 'recommends', 'requires', 'suggests', 'supplements']

    for dep in dep_types:
        ret = utils.run(['tdnf',
                         'repoquery',
                         '--file',
                         '/usr/lib/repoquery/tdnf-repoquery-{}'.format(dep)
                         ])
        assert(ret['retval'] == 0)
        assert('tdnf-repoquery-{}'.format(dep) in '\n'.join(ret['stdout']))


def test_available(utils):
    ret = utils.run(['tdnf',
                     'repoquery',
                     '--available'])
    assert(ret['retval'] == 0)
    assert(BASE_PKG in '\n'.join(ret['stdout']))


def test_installed(utils):
    ret = utils.run(['tdnf',
                     'install', '-y', '--nogpgcheck',
                     BASE_PKG])
    assert(ret['retval'] == 0)

    ret = utils.run(['tdnf',
                     'repoquery',
                     '--installed'])
    assert(ret['retval'] == 0)
    assert(BASE_PKG in '\n'.join(ret['stdout']))


def test_extras(utils):
    ret = utils.run(['tdnf',
                     'repoquery',
                     '--extras'])
    assert(ret['retval'] == 0)
    # we are using just the 'photon-test repo,
    # so any real system package is 'extra'
    assert('glibc' in '\n'.join(ret['stdout']))


def test_upgrades(utils):
    pkg_low = "{}-{}".format(utils.config["mulversion_pkgname"], utils.config["mulversion_lower"])
    pkg_high = "{}-{}".format(utils.config["mulversion_pkgname"], utils.config["mulversion_higher"])

    ret = utils.run(['tdnf',
                     'install', '-y', '--nogpgcheck',
                     pkg_low])
    assert(ret['retval'] == 0)

    ret = utils.run(['tdnf',
                     'repoquery',
                     '--upgrades'])
    assert(ret['retval'] == 0)
    assert(pkg_high in '\n'.join(ret['stdout']))


def test_changelog(utils):
    ret = utils.run(['tdnf',
                     'repoquery',
                     '--changelogs',
                     'tdnf-repoquery-changelog'])
    assert(ret['retval'] == 0)

    # tests changelog text, author and date
    output = '\n'.join(ret['stdout'])
    assert("needle in a haystack" in output)
    assert("John Doe" in output)
    assert("Wed Jan 01 2020" in output)


def test_source(utils):
    # any package should do that has
    # the same name as the source
    pkgname = 'tdnf-repoquery-base'

    ret = utils.run(['tdnf',
                     'repoquery',
                     '--source',
                     pkgname])
    assert(ret['retval'] == 0)
    output = '\n'.join(ret['stdout'])
    assert(pkgname in output)
    assert('src' in output)
    assert('x86_64' not in output)


# each package has a file with its name
def test_list1(utils):
    dep = 'requires'
    ret = utils.run_memcheck(['tdnf',
                              'repoquery',
                              '--list',
                              'tdnf-repoquery-{}'.format(dep)
                              ])
    assert(ret['retval'] == 0)


# fail with ore than one pkg spec
def test_two_args(utils):
    ret = utils.run(['tdnf',
                     'repoquery',
                     'foo',
                     'bar'])
    assert(ret['retval'] == 902)
