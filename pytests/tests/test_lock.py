#
# Copyright (C) 2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import os
import time
import pytest
from threading import Thread
from subprocess import Popen, PIPE

proc = None
t1_started = False
t2_started = False
t3_started = False

t1_failed = False
t2_failed = False
t3_failed = False

expected_str = 'WARNING: failed to acquire lock on: /var/run/.tdnf-instance-lockfile, retrying ...'


@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    pkgname = utils.config["sglversion_pkgname"]
    utils.run(['tdnf', 'erase', '-y', pkgname])


def run_tdnf_blocking_cmd(utils, pkgname):
    try:
        cmd = ['tdnf', 'install', pkgname]
        utils._decorate_tdnf_cmd_for_test(cmd)
        print(cmd)
        global t1_started
        global proc
        proc = Popen(cmd, stdout=PIPE, stderr=PIPE, stdin=PIPE)
        t1_started = True
        proc.wait()
        out, err = proc.communicate()
        out = out.decode().strip().split('\n')
        err = err.decode().strip().split('\n')
        print('\n\n\n', out, err)
        assert 'Installing:' in out
    except Exception:
        global t1_failed
        t1_failed = True


def run_tdnf_search_cmd(utils, pkgname):
    cmd = ['tdnf', 'search', pkgname]
    utils._decorate_tdnf_cmd_for_test(cmd)
    global t2_started
    t2_started = True
    ret = utils.run(cmd)  # this gets blocked till install finishes
    try:
        assert expected_str in ret['stderr']
        assert 'tdnf-test-one : basic install test file.' in ret['stdout']
    except Exception:
        global t2_failed
        t2_failed = True


def run_tdnf_info_cmd(utils, pkgname):
    cmd = ['tdnf', 'info', pkgname]
    utils._decorate_tdnf_cmd_for_test(cmd)
    global t3_started
    t3_started = True
    ret = utils.run(cmd)  # this gets blocked till install finishes
    print(ret['stdout'])
    try:
        assert expected_str in ret['stderr']
        assert 'Name          : tdnf-test-one' in ret['stdout']
    except Exception:
        global t3_failed
        t3_failed = True


def test_lock_basic(utils):
    pkgname = utils.config["sglversion_pkgname"]
    utils.run(['tdnf', 'erase', '-y', pkgname])

    t1 = Thread(target=run_tdnf_blocking_cmd, args=(utils, pkgname, ))
    t2 = Thread(target=run_tdnf_search_cmd, args=(utils, pkgname, ))
    t3 = Thread(target=run_tdnf_info_cmd, args=(utils, pkgname, ))

    t1.start()
    while not t1_started:
        time.sleep(0.5)
    print(proc.pid)

    t2.start()
    t3.start()
    while not t2_started or not t3_started:
        time.sleep(0.5)

    time.sleep(1)

    # write 'n' to stdin of install pid
    os.system("echo n > /proc/" + str(proc.pid) + "/fd/0")
    t1.join()
    t2.join()
    t3.join()

    assert not t1_failed and not t2_failed and not t3_failed
