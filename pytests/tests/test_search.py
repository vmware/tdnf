#
# Copyright (C) 2019-2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import pytest


@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    pass


def test_search_no_arg(utils):
    ret = utils.run(['tdnf', 'search'])
    assert ret['retval'] == 1011


def test_search_invalid_arg(utils):
    ret = utils.run(['tdnf', 'search', 'invalid_arg'])
    assert "No search results found for 'invalid_arg'" in "\n".join(ret['stderr'])
    assert ret['retval'] == 1011


def test_search_single(utils):
    pkgname = utils.config["sglversion_pkgname"]
    ret = utils.run(['tdnf', 'search', pkgname])
    assert ret['retval'] == 0


def test_search_multiple(utils):
    sglpkgname = utils.config["sglversion_pkgname"]
    mulpkgname = utils.config["mulversion_pkgname"]
    ret = utils.run(['tdnf', 'search', sglpkgname, mulpkgname])
    assert ret['retval'] == 0


def test_search_valid_with_invalid(utils):
    sglpkgname = utils.config["sglversion_pkgname"]

    ret = utils.run(f"tdnf search {sglpkgname} invalid1")
    assert ret['retval'] == 1599
    assert "No search results found for 'invalid1'" in "\n".join(ret['stderr'])
    assert "tdnf-test-one : basic install test file" in "\n".join(ret['stdout'])

    ret = utils.run(f"tdnf search {sglpkgname} invalid1 invalid2")
    assert ret['retval'] == 1599
    assert "No search results found for 'invalid1'" in "\n".join(ret['stderr'])
    assert "No search results found for 'invalid2'" in "\n".join(ret['stderr'])
    assert "tdnf-test-one : basic install test file" in "\n".join(ret['stdout'])
