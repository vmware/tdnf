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
    # Add repos for glob testing
    repofile_example = os.path.join(utils.config['repo_path'], 'yum.repos.d', 'example.repo')
    utils.edit_config(
        {
            'name': 'Example Repo',
            'enabled': '1',
            'baseurl': 'http://pkgs.example.org/example'
        },
        section='example-test',
        filename=repofile_example
    )
    utils.edit_config(
        {
            'name': 'Example Debug Repo',
            'enabled': '0',
            'baseurl': 'http://pkgs.example.org/example-debug'
        },
        section='example-debug',
        filename=repofile_example
    )
    utils.edit_config(
        {
            'name': 'Example Updates Repo',
            'enabled': '0',
            'baseurl': 'http://pkgs.example.org/example-updates'
        },
        section='example-updates',
        filename=repofile_example
    )
    yield
    teardown_test(utils)


def teardown_test(utils):
    for fn in ["foo", "bar", "test", "example"]:
        fn = os.path.join(utils.config["repo_path"], "yum.repos.d", f"{fn}.repo")
        if os.path.isfile(fn):
            os.remove(fn)


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
    os.remove(os.path.join(utils.config['repo_path'], "yum.repos.d", 'test1.repo'))


# Test comma-separated repo names for enablerepo
def test_repolist_enable_comma_separated(utils):
    ret = utils.run(['tdnf', 'repolist', '--enablerepo=foo-debug,bar', '-j'])
    assert ret['retval'] == 0
    repolist = json.loads("\n".join(ret['stdout']))
    assert find_repo(repolist, 'foo')
    assert find_repo(repolist, 'foo-debug')
    assert find_repo(repolist, 'bar')


# Test comma-separated repo names for disablerepo
def test_repolist_disable_comma_separated(utils):
    ret = utils.run(['tdnf', 'repolist', '--disablerepo=foo,bar', '-j'])
    assert ret['retval'] == 0
    repolist = json.loads("\n".join(ret['stdout']))
    assert not find_repo(repolist, 'foo')
    assert not find_repo(repolist, 'bar')
    # foo-debug should still be disabled (not enabled by default)
    assert not find_repo(repolist, 'foo-debug')


# Test comma-separated repo names for repoid
def test_repolist_repoid_comma_separated(utils):
    ret = utils.run(['tdnf', 'repolist', '--repoid=foo-debug,bar', '-j'])
    assert ret['retval'] == 0
    repolist = json.loads("\n".join(ret['stdout']))
    # repoid disables all repos first, then enables only the specified ones
    assert find_repo(repolist, 'foo-debug')
    assert find_repo(repolist, 'bar')
    # foo should be disabled (repoid disables all others)
    assert not find_repo(repolist, 'foo')
    # example repos should be disabled
    assert not find_repo(repolist, 'example-test')
    assert not find_repo(repolist, 'example-debug')


# Test comma-separated repo names for repo (alias for repoid)
def test_repolist_repo_comma_separated(utils):
    ret = utils.run(['tdnf', 'repolist', '--repo=foo-debug,bar', '-j'])
    assert ret['retval'] == 0
    repolist = json.loads("\n".join(ret['stdout']))
    # repo disables all repos first, then enables only the specified ones
    assert find_repo(repolist, 'foo-debug')
    assert find_repo(repolist, 'bar')
    # foo should be disabled (repo disables all others)
    assert not find_repo(repolist, 'foo')
    # example repos should be disabled
    assert not find_repo(repolist, 'example-test')
    assert not find_repo(repolist, 'example-debug')


# Test glob pattern for enablerepo
def test_repolist_enable_glob(utils):
    ret = utils.run(['tdnf', 'repolist', '--enablerepo=example*', '-j'])
    assert ret['retval'] == 0
    repolist = json.loads("\n".join(ret['stdout']))
    assert find_repo(repolist, 'example-test')
    assert find_repo(repolist, 'example-debug')
    assert find_repo(repolist, 'example-updates')


# Test glob pattern for disablerepo
def test_repolist_disable_glob(utils):
    ret = utils.run(['tdnf', 'repolist', '--disablerepo=foo*', '-j'])
    assert ret['retval'] == 0
    repolist = json.loads("\n".join(ret['stdout']))
    assert not find_repo(repolist, 'foo')
    assert not find_repo(repolist, 'foo-debug')
    # bar should still be enabled
    assert find_repo(repolist, 'bar')


# Test comma-separated globs for enablerepo
def test_repolist_enable_comma_separated_globs(utils):
    ret = utils.run(['tdnf', 'repolist', '--enablerepo=example*,foo*', '-j'])
    assert ret['retval'] == 0
    repolist = json.loads("\n".join(ret['stdout']))
    assert find_repo(repolist, 'example-test')
    assert find_repo(repolist, 'example-debug')
    assert find_repo(repolist, 'example-updates')
    assert find_repo(repolist, 'foo')
    assert find_repo(repolist, 'foo-debug')


# Test comma-separated globs for disablerepo
def test_repolist_disable_comma_separated_globs(utils):
    ret = utils.run(['tdnf', 'repolist', '--disablerepo=example*,foo*', '-j'])
    assert ret['retval'] == 0
    repolist = json.loads("\n".join(ret['stdout']))
    assert not find_repo(repolist, 'example-test')
    assert not find_repo(repolist, 'example-debug')
    assert not find_repo(repolist, 'example-updates')
    assert not find_repo(repolist, 'foo')
    assert not find_repo(repolist, 'foo-debug')
    # bar should still be enabled
    assert find_repo(repolist, 'bar')


# Test mixed comma-separated (globs and non-globs) for enablerepo
def test_repolist_enable_mixed(utils):
    ret = utils.run(['tdnf', 'repolist', '--enablerepo=example*,bar', '-j'])
    assert ret['retval'] == 0
    repolist = json.loads("\n".join(ret['stdout']))
    assert find_repo(repolist, 'example-test')
    assert find_repo(repolist, 'example-debug')
    assert find_repo(repolist, 'example-updates')
    assert find_repo(repolist, 'bar')


# Test mixed comma-separated (globs and non-globs) for disablerepo
def test_repolist_disable_mixed(utils):
    ret = utils.run(['tdnf', 'repolist', '--disablerepo=foo*,bar', '-j'])
    assert ret['retval'] == 0
    repolist = json.loads("\n".join(ret['stdout']))
    assert not find_repo(repolist, 'foo')
    assert not find_repo(repolist, 'foo-debug')
    assert not find_repo(repolist, 'bar')
