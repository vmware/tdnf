#
# Copyright (C) 2024 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import os
from pathlib import Path
import shutil
import tempfile
import pytest


@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    pass


# builddep with no argument should fail
def test_builddep_no_args(utils):
    ret = utils.run(['tdnf', 'builddep'])
    assert ret['retval'] != 0


# builddep with an unrecognised argument that is not a spec/srpm/package
# should fail (treated as a source package name and not found in repos)
def test_builddep_invalid_package(utils):
    ret = utils.run(['tdnf', 'builddep', 'this-package-does-not-exist'])
    assert ret['retval'] != 0


# builddep with a nonexistent spec file path should fail gracefully
def test_builddep_nonexistent_spec(utils):
    ret = utils.run(['tdnf', 'builddep', '/tmp/nonexistent-file.spec'])
    assert ret['retval'] != 0


# builddep with a minimal spec file (no BuildRequires) should succeed and
# report nothing to install
def test_builddep_empty_spec(utils):
    # Create a minimal spec file with no BuildRequires
    spec_content = """\
Name:           tdnf-test-empty-spec
Version:        1.0
Release:        1
Summary:        Test package with no build dependencies
License:        GPLv2

%description
Test spec for builddep with no BuildRequires.
"""
    with tempfile.NamedTemporaryFile(suffix='.spec', mode='w',
                                     delete=False) as f:
        f.write(spec_content)
        spec_path = f.name

    try:
        # rpmspec may not be available; skip if it's not
        if not shutil.which('rpmspec'):
            pytest.skip('rpmspec not available')

        ret = utils.run(['tdnf', 'builddep', spec_path])
        # Should succeed (nothing to do is not an error for builddep)
        assert ret['retval'] == 0 or ret['retval'] == 905  # NOTHING_TO_DO
    finally:
        os.unlink(spec_path)


# builddep via a source package name resolves BuildRequires through the solver
def test_builddep_source_package(utils):
    # The test repo may or may not have source packages; if it does not,
    # the command should fail with "no match" rather than crash.
    ret = utils.run(['tdnf', 'builddep', 'tdnf-test-one'])
    # Either succeeds (deps installed) or fails cleanly (no source pkg found)
    assert ret['retval'] in (0, 905, 1011, 1301)  # ok, nothing-to-do, no-match, solv-fail


@pytest.fixture
def spec_file(tmp_path):
    if not os.access('/usr/bin/rpmspec', os.X_OK):
        pytest.skip('rpmspec not available')

    def write_spec(name, requires=None):
        path = tmp_path / name
        dependency = 'BuildRequires: {}\n'.format(requires) if requires else ''
        path.write_text(
            'Name: tdnf-test-builddep\n'
            'Version: 1.0\n'
            'Release: 1\n'
            'Summary: Build dependency test\n'
            'License: GPLv2\n' + dependency +
            '\n%description\nBuild dependency test.\n'
        )
        return path

    return write_spec


@pytest.mark.parametrize('name', ["quoted'name.spec", 'space in name.spec',
                                  '-leading-dash.spec'])
def test_builddep_literal_spec_filename(utils, spec_file, name):
    spec = spec_file(name)
    ret = utils.run(['tdnf', 'builddep', '--', spec.name], cwd=str(spec.parent))
    assert ret['retval'] == 0
    assert any('No build dependencies found' in line for line in ret['stdout'])


def test_builddep_invalid_spec(utils, spec_file):
    spec = spec_file('invalid.spec')
    spec.write_text('This is not a valid RPM spec file.\n')
    ret = utils.run(['tdnf', 'builddep', str(spec)])
    assert ret['retval'] != 0
    assert any('rpmspec failed to parse' in line for line in ret['stderr'])
    assert not any('No build dependencies found' in line for line in ret['stdout'])


def test_builddep_satisfied_spec_does_not_skip_later_spec(utils, spec_file):
    if utils.run(['rpm', '-q', 'bash'])['retval']:
        pytest.skip('bash must be installed for this regression')
    satisfied = spec_file('satisfied.spec', 'bash')
    missing = 'tdnf-test-builddep-missing-dependency'
    unsatisfied = spec_file('unsatisfied.spec', missing)

    ret = utils.run(['tdnf', '--dryrun', 'builddep', str(satisfied), str(unsatisfied)])
    assert ret['retval'] == 1011
    assert missing in '\n'.join(ret['stdout'] + ret['stderr'])


def test_builddep_parses_all_specs_before_resolving(utils, spec_file):
    first = spec_file('first.spec', 'tdnf-test-builddep-missing-dependency')
    invalid = spec_file('invalid.spec')
    invalid.write_text('This is not a valid RPM spec file.\n')

    ret = utils.run(['tdnf', '--dryrun', 'builddep', str(first), str(invalid)])
    assert ret['retval'] != 0
    assert any('rpmspec failed to parse' in line for line in ret['stderr'])


@pytest.mark.parametrize('spec_first', [True, False])
def test_builddep_mixed_spec_and_srpm(utils, spec_file, spec_first):
    if utils.run(['rpm', '-q', 'bash'])['retval']:
        pytest.skip('bash must be installed for this regression')
    srpm_dir = Path(utils.config['repo_path']) / 'photon-test-src' / 'SRPMS'
    srpm = next(srpm_dir.glob(utils.config['sglversion_pkgname'] + '-*.src.rpm'))
    spec = spec_file('satisfied.spec', 'bash')
    inputs = [str(spec), str(srpm)]
    if not spec_first:
        inputs.reverse()

    baseline = utils.run(['tdnf', '--dryrun', '--nogpgcheck', 'builddep', str(srpm)])
    assert baseline['retval'] == 0
    ret = utils.run(['tdnf', '--dryrun', '--nogpgcheck', 'builddep'] + inputs)
    assert ret['retval'] == 0
