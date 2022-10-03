#
# Copyright (C) 2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#
#   Author: Oliver Kurth <okurth@vmware.com>

import os
import pytest
import glob
import fnmatch

REPOFILENAME = "photon-skip.repo"
REPOID = "photon-skip"

pkg0 = "tdnf-conflict-file0"
pkg1 = "tdnf-conflict-file1"


@pytest.fixture(scope='function', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    os.remove(os.path.join(utils.config['repo_path'], "yum.repos.d", REPOFILENAME))
    utils.run(['tdnf', 'erase', '-y', pkg0])
    utils.run(['tdnf', 'erase', '-y', pkg1])


def generate_repofile_skip_md(utils, newconfig, repoid, mdpart, value):
    orig_repoconf = os.path.join(utils.config['repo_path'], "yum.repos.d", "photon-test.repo")

    option = "skip_md_{}".format(mdpart)

    # no easy way to rename a section with configparser
    with open(orig_repoconf, 'r') as fin:
        with open(newconfig, 'w') as fout:
            for line in fin:
                if line.startswith('['):
                    fout.write("[{}]\n".format(repoid))
                else:
                    fout.write(line)

    utils.edit_config({
        option: '1' if value else '0',
        'enabled': '0'},  # we will enable this with the --repoid option
        repo=REPOID)


def get_cache_dir(utils):
    return utils.tdnf_config.get('main', 'cachedir')


def find_cache_dir(utils, reponame):
    cache_dir = get_cache_dir(utils)
    for f in os.listdir(cache_dir):
        if fnmatch.fnmatch(f, '{}-*'.format(reponame)):
            return os.path.join(cache_dir, f)
    return None


# enable/disable md part, expect/do not expect download of the associated file
def check_skip_md_part(utils, mdpart, skipped):
    repoconf = os.path.join(utils.config['repo_path'], "yum.repos.d", REPOFILENAME)
    generate_repofile_skip_md(utils, repoconf, REPOID, mdpart, skipped)
    utils.run(['tdnf', '--repoid={}'.format(REPOID), 'clean', 'all'])
    utils.run(['tdnf', '--repoid={}'.format(REPOID), 'makecache'])

    md_dir = os.path.join(find_cache_dir(utils, REPOID), 'repodata')
    assert (len(glob.glob('{}/*{}*'.format(md_dir, mdpart))) == 0) == skipped


def test_skip_md_parts(utils):
    # we do not generate updateinfo in our tests
    #    for mdpart in ['filelists', 'updateinfo', 'other']:
    for mdpart in ['filelists', 'other']:
        check_skip_md_part(utils, mdpart, True)
        check_skip_md_part(utils, mdpart, False)


# even with filelists dropped, trying to install packages with conflicting files
# should still fail
def test_install_conflict_file(utils):
    repoconf = os.path.join(utils.config['repo_path'], "yum.repos.d", REPOFILENAME)
    generate_repofile_skip_md(utils, repoconf, REPOID, 'filelists', True)

    ret = utils.run(['tdnf', '--repoid={}'.format(REPOID), 'install', '-y', '--nogpgcheck', pkg0])
    print(ret)
    assert utils.check_package(pkg0)

    ret = utils.run(['tdnf', '--repoid={}'.format(REPOID), 'install', '-y', '--nogpgcheck', pkg1])
    print(ret)
    assert ret['retval'] == 1525
    assert not utils.check_package(pkg1)


def test_install_conflict_file_atonce(utils):
    repoconf = os.path.join(utils.config['repo_path'], "yum.repos.d", REPOFILENAME)
    generate_repofile_skip_md(utils, repoconf, REPOID, 'filelists', True)

    ret = utils.run(['tdnf', '--repoid={}'.format(REPOID), 'install', '-y', '--nogpgcheck', pkg0, pkg1])
    print(ret)
    assert ret['retval'] == 1525
    assert not utils.check_package(pkg0)
    assert not utils.check_package(pkg1)
