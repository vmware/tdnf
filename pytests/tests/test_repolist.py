#
# Copyright (C) 2019-2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import pytest
import os
import json


@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    # test multiple repos in one file
    repofile_foo = os.path.join(utils.config['repo_path'], 'yum.repos.d', 'foo.repo')
    utils.edit_config(
        {
            'name': 'Foo Repo',
            'enabled': '1',
            'baseurl': 'http://pkgs.foo.org/foo'
        },
        section='foo',
        filename=repofile_foo
    )
    utils.edit_config(
        {
            'name': 'Foo Debug Repo',
            'enabled': '0',
            'baseurl': 'http://pkgs.foo.org/foo-debug'
        },
        section='foo-debug',
        filename=repofile_foo
    )
    repofile_bar = os.path.join(utils.config['repo_path'], 'yum.repos.d', 'bar.repo')
    utils.edit_config(
        {
            'name': 'Bar Repo',
            'enabled': '1',
            'baseurl': 'http://pkgs.bar.org/bar'
        },
        section='bar',
        filename=repofile_bar
    )
    yield
    teardown_test(utils)


def teardown_test(utils):
    os.remove(os.path.join(utils.config['repo_path'], 'yum.repos.d', 'foo.repo'))
    os.remove(os.path.join(utils.config['repo_path'], 'yum.repos.d', 'bar.repo'))
    os.remove(os.path.join(utils.config['repo_path'], "yum.repos.d", 'test.repo'))
    os.remove(os.path.join(utils.config['repo_path'], "yum.repos.d", 'test1.repo'))


def find_repo(repolist, id):
    for repo in repolist:
        if repo['Repo'] == id:
            return True
    return False


def test_repolist(utils):
    ret = utils.run(['tdnf', 'repolist'])
    assert ret['retval'] == 0


# -j returns a list of repos. Easier to parse.
def test_repolist_json(utils):
    ret = utils.run(['tdnf', 'repolist', '-j'])
    assert ret['retval'] == 0
    repolist = json.loads("\n".join(ret['stdout']))
    assert find_repo(repolist, 'foo')
    assert not find_repo(repolist, 'foo-debug')
    assert find_repo(repolist, 'bar')


# disabled repo should be listed when we enable it on the command line
def test_repolist_json_enable_one(utils):
    ret = utils.run(['tdnf', 'repolist', '--enablerepo=foo-debug', '-j'])
    assert ret['retval'] == 0
    repolist = json.loads("\n".join(ret['stdout']))
    assert find_repo(repolist, 'foo')
    assert find_repo(repolist, 'foo-debug')
    assert find_repo(repolist, 'bar')


def test_repolist_all(utils):
    ret = utils.run(['tdnf', 'repolist', 'all'])
    assert ret['retval'] == 0


def test_repolist_json_all(utils):
    ret = utils.run(['tdnf', 'repolist', 'all', '-j'])
    assert ret['retval'] == 0
    repolist = json.loads("\n".join(ret['stdout']))
    assert find_repo(repolist, 'foo')
    assert find_repo(repolist, 'foo-debug')
    assert find_repo(repolist, 'bar')


def test_repolist_enabled(utils):
    ret = utils.run(['tdnf', 'repolist', 'enabled'])
    assert ret['retval'] == 0


def test_repolist_disabled(utils):
    ret = utils.run(['tdnf', 'repolist', 'disabled'])
    assert ret['retval'] == 0


def test_repolist_json_disabled(utils):
    ret = utils.run(['tdnf', 'repolist', 'disabled', '-j'])
    assert ret['retval'] == 0
    repolist = json.loads("\n".join(ret['stdout']))
    assert not find_repo(repolist, 'foo')
    assert find_repo(repolist, 'foo-debug')
    assert not find_repo(repolist, 'bar')


def test_repolist_invalid(utils):
    ret = utils.run(['tdnf', 'repolist', 'invalid_repo'])
    assert ret['retval'] == 901


# memcheck
def test_repolist_memcheck(utils):
    ret = utils.run_memcheck(['tdnf', 'repolist'])
    assert ret['retval'] == 0


# multiple repoid
def test_multiple_repoid(utils):
    reponame = 'test.repo'
    repofile_test = os.path.join(utils.config['repo_path'], 'yum.repos.d', reponame)
    utils.edit_config(
        {
            'name': 'Test Repo',
            'enabled': '1',
            'baseurl': 'http://pkgs.test.org/test'
        },
        section='test',
        filename=repofile_test
    )

    reponame = 'test1.repo'
    repofile_test1 = os.path.join(utils.config['repo_path'], 'yum.repos.d', reponame)
    utils.edit_config(
        {
            'name': 'Test Repo',
            'enabled': '1',
            'baseurl': 'http://pkgs.test1.org/test1'
        },
        section='test',
        filename=repofile_test1
    )

    ret = utils.run(['tdnf',
                     '--disablerepo=*', '--enablerepo={}'.format(reponame),
                     'makecache'])
    assert ret['retval'] == 1037
