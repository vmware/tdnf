#
# Copyright (C) 2019-2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import json
import pytest


@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    pass


def test_search_no_arg(utils):
    ret = utils.run(['tdnf', 'search'])
    assert ret['retval'] == 1599


def test_search_invalid_arg(utils):
    ret = utils.run(['tdnf', 'search', 'invalid_arg'])
    assert ret['retval'] == 1599


def test_search_single(utils):
    ret = utils.run(['tdnf', 'search', 'tdnf'])
    assert ret['retval'] == 0


def test_search_multiple(utils):
    ret = utils.run(['tdnf', 'search', 'tdnf', 'wget', 'gzip'])
    assert ret['retval'] == 0


def test_search_multiple_invalid(utils):
    # default: succeed if at least one term matches
    ret = utils.run(['tdnf', 'search', 'tdnf', 'invalid_arg'])
    assert ret['retval'] == 0


def test_search_multiple_require_all(utils):
    opt = '--setopt=require_all_matches=1'

    ret = utils.run(['tdnf', opt, 'search',
                     utils.config["mulversion_pkgname"],
                     utils.config["sglversion_pkgname"]])
    assert ret['retval'] == 0

    # fail if any of the terms has no match, but still list the others
    for args in [['tdnf', 'invalid_arg'], ['invalid_arg', 'tdnf']]:
        ret = utils.run(['tdnf', opt, 'search'] + args)
        assert ret['retval'] == 1599
        assert any(line.startswith('tdnf') for line in ret['stdout'])
        assert "No match found for 'invalid_arg'" in '\n'.join(ret['stderr'])
        assert "No match found for 'tdnf'" not in '\n'.join(ret['stderr'])

    ret = utils.run(['tdnf', opt, 'search', 'invalid_arg', 'invalid_arg2'])
    assert ret['retval'] == 1599
    assert "No match found for 'invalid_arg'" in '\n'.join(ret['stderr'])
    assert "No match found for 'invalid_arg2'" in '\n'.join(ret['stderr'])


def test_search_multiple_require_all_json(utils):
    opt = '--setopt=require_all_matches=1'

    # json output does not fail for terms without a match
    ret = utils.run(['tdnf', opt, '-j', 'search', 'tdnf', 'invalid_arg'])
    assert ret['retval'] == 0
    assert "No match found for 'invalid_arg'" not in '\n'.join(ret['stderr'])
    pkgs = json.loads("\n".join(ret['stdout']))
    assert any(pkg['Name'].startswith('tdnf') for pkg in pkgs)

    ret = utils.run(['tdnf', opt, '-j', 'search', 'invalid_arg', 'invalid_arg2'])
    assert ret['retval'] == 0
    assert json.loads("\n".join(ret['stdout'])) == []
