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


REPOFILENAME = "photon-skip.repo"
REPOID = "photon-skip"


@pytest.fixture(scope='function', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    os.remove(os.path.join(utils.config['repo_path'], "yum.repos.d", REPOFILENAME))
    pass


def generate_repofile_skip_md(utils, newconfig, repoid, mdpart, value):
    orig_repoconf = os.path.join(utils.config['repo_path'], "yum.repos.d", "photon-test.repo")
    option = "skip_md_{}".format(mdpart)

    with open(orig_repoconf, 'r') as fin:
        with open(newconfig, 'w') as fout:
            for line in fin:
                if line.startswith('['):
                    fout.write("[{}]".format(repoid))
                elif line.startswith('enabled'):
                    # we will enable this with the --repoid option
                    fout.write('enabled=0\n')
                elif not line.startswith(option):
                    fout.write(line)
            fout.write('{}={}\n'.format(option, '1' if value else '0'))

    with open(newconfig, 'r') as fin:
        for line in fin:
            print(line)


def get_cache_dir(utils):
    conffile = os.path.join(utils.config['repo_path'], 'tdnf.conf')
    with open(conffile, 'r') as fin:
        for line in fin:
            if line.startswith("cachedir"):
                return line.split('=')[1].strip()
    return '/var/cache/tdnf/'


# enable/disable md part, expect/do not expect download of the associated file
def check_skip_md_part(utils, mdpart, skipped):
    repoconf = os.path.join(utils.config['repo_path'], "yum.repos.d", REPOFILENAME)
    generate_repofile_skip_md(utils, repoconf, REPOID, mdpart, skipped)
    utils.run(['tdnf', '--repoid={}'.format(REPOID), 'clean', 'all'])
    utils.run(['tdnf', '--repoid={}'.format(REPOID), 'makecache'])

    md_dir = os.path.join(get_cache_dir(utils), REPOID, 'repodata')
    assert((len(glob.glob('{}/*{}*'.format(md_dir, mdpart))) == 0) == skipped)


def test_skip_md_filelists(utils):
    # we do not generate updateinfo in our tests
    #    for mdpart in ['filelists', 'updateinfo', 'other']:
    for mdpart in ['filelists', 'other']:
        check_skip_md_part(utils, mdpart, True)
        check_skip_md_part(utils, mdpart, False)
