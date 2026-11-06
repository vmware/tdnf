#
# Copyright (C) 2026 Broadcom, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import pytest

ERROR_TDNF_SOLV_FAILED = 1301

# tdnf-test-longdep-req requires a capability with a name longer than 256
# characters. Its only provider, tdnf-test-longdep-prov, cannot be
# installed, so with --verbose the solver problem includes a "requires"
# rule for the long name.
PKGNAME = 'tdnf-test-longdep-req'
LONG_DEP = 'tdnf-test-longdep-' + 'x' * 300

# --skipconflicts makes problem reporting look up the required name of
# "requires" rules instead of printing them unconditionally.
CMD = ['tdnf', 'install', '-y', '--nogpgcheck', '--skipconflicts', '--verbose', PKGNAME]


@pytest.fixture(scope='function', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    utils.run(['tdnf', 'erase', '-y', PKGNAME, 'tdnf-test-longdep-prov'])


def test_long_dependency_problem_reported(utils):
    ret = utils.run(CMD)
    output = ret['stdout'] + ret['stderr']

    assert ret['retval'] == ERROR_TDNF_SOLV_FAILED
    assert not utils.check_package(PKGNAME)
    # the complete name was looked up and found to have a provider, so the
    # "requires" rule is skipped and only the root cause is reported
    assert any("capability match for '{}'".format(LONG_DEP) in line for line in output)
    assert not any('requires {}'.format(LONG_DEP) in line for line in output)
    assert any('nothing provides tdnf-test-longdep-missing' in line for line in output)


def test_long_dependency_problem_memcheck(utils):
    utils.run_memcheck(CMD)
