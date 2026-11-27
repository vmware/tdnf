#
# Copyright (C) 2024 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import os
import shlex
import pytest


@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    yield


# needs-restarting runs without error on a normal system
def test_needs_restarting_basic(utils):
    ret = utils.run(['tdnf', 'needs-restarting'])
    assert ret['retval'] in (0, 1)


# -r flag (reboot required check) should run without crashing;
# returns 0 if no reboot is needed, 1 if a reboot is required.
def test_needs_restarting_reboot_check(utils):
    ret = utils.run(['tdnf', 'needs-restarting', '-r'])
    assert ret['retval'] in (0, 1)


# output is either the "no processes need restarting" message or a list of PIDs
def test_needs_restarting_output_format(utils):
    ret = utils.run(['tdnf', 'needs-restarting'])
    assert ret['retval'] in (0, 1)
    combined = '\n'.join(ret['stdout'] + ret['stderr'])
    # Either a human-readable "no processes" message or lines of "PID name"
    if 'No processes' not in combined:
        # Each output line should be non-empty
        for line in ret['stdout']:
            if line.strip():
                parts = line.split()
                # First part should be numeric PID
                assert parts[0].isdigit(), (
                    "Expected numeric PID in output, got: {}".format(line))


@pytest.mark.parametrize('reboot_needed', [False, True])
def test_needs_restarting_reboot_exit_status(utils, tmp_path, reboot_needed):
    if not os.access('/usr/bin/rpm', os.X_OK):
        pytest.skip('rpm not available')
    release = 'review-different-release' if reboot_needed else os.uname().release
    rpm = tmp_path / 'rpm'
    rpm.write_text(
        '#!/bin/sh\n'
        'if [ "$1" = "-qf" ]; then\n'
        '    printf "%s\\n" linux\n'
        'else\n'
        '    printf "%s\\n" ' + shlex.quote('linux-' + release) + '\n'
        'fi\n')
    rpm.chmod(0o755)
    env = dict(os.environ, PATH=str(tmp_path) + os.pathsep + os.environ['PATH'])
    ret = utils.run(['tdnf', 'needs-restarting', '-r'], env=env)
    assert ret['retval'] == int(reboot_needed)
    assert not ret['stderr']
    output = '\n'.join(ret['stdout'])
    assert ('Reboot is required' if reboot_needed else 'No reboot required') in output
