#
# Copyright (C) 2019-2024 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.

import pytest


@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    pkgname = utils.config['sglversion_pkgname']
    if utils.check_package(pkgname):
        utils.run(['tdnf', 'erase', '-y', pkgname])
    yield
    utils.run(['tdnf', 'erase', '-y', pkgname])


# dry-run install must not install the package
def test_dry_run_install_does_not_install(utils):
    pkgname = utils.config['sglversion_pkgname']
    ret = utils.run(['tdnf', '--dryrun', 'install', pkgname])
    assert ret['retval'] == 0
    assert not utils.check_package(pkgname)


# dry-run install must show what would be installed
def test_dry_run_install_shows_plan(utils):
    pkgname = utils.config['sglversion_pkgname']
    ret = utils.run(['tdnf', '--dryrun', 'install', pkgname])
    assert ret['retval'] == 0
    output = '\n'.join(ret['stdout'] + ret['stderr'])
    assert pkgname in output


# dry-run install must print the completion message
def test_dry_run_install_completion_message(utils):
    pkgname = utils.config['sglversion_pkgname']
    ret = utils.run(['tdnf', '--dryrun', 'install', pkgname])
    assert ret['retval'] == 0
    output = '\n'.join(ret['stdout'] + ret['stderr'])
    assert 'Dry run complete. Transaction is feasible. Run without --dryrun to apply the above changes.' in output


# dry-run on an already-installed package must report nothing to do
def test_dry_run_install_already_installed(utils):
    pkgname = utils.config['sglversion_pkgname']
    utils.install_package(pkgname)
    assert utils.check_package(pkgname)
    ret = utils.run(['tdnf', '--dryrun', 'install', pkgname])
    assert ret['retval'] == 0
    output = '\n'.join(ret['stdout'] + ret['stderr'])
    assert 'Nothing to do' in output
    utils.erase_package(pkgname)


# dry-run erase must not remove the package
def test_dry_run_erase_does_not_erase(utils):
    pkgname = utils.config['sglversion_pkgname']
    utils.install_package(pkgname)
    assert utils.check_package(pkgname)
    ret = utils.run(['tdnf', '--dryrun', 'erase', pkgname])
    assert ret['retval'] == 0
    assert utils.check_package(pkgname)
    utils.erase_package(pkgname)


# dry-run erase must show the removal plan
def test_dry_run_erase_shows_plan(utils):
    pkgname = utils.config['sglversion_pkgname']
    utils.install_package(pkgname)
    assert utils.check_package(pkgname)
    ret = utils.run(['tdnf', '--dryrun', 'erase', pkgname])
    assert ret['retval'] == 0
    output = '\n'.join(ret['stdout'] + ret['stderr'])
    assert pkgname in output
    utils.erase_package(pkgname)


# dry-run on a non-existent package must fail
def test_dry_run_install_nonexistent(utils):
    ret = utils.run(['tdnf', '--dryrun', 'install', 'no-such-package-xyz'])
    assert ret['retval'] != 0


# dry-run must not download any RPMs to cache
def test_dry_run_no_download(utils):
    pkgname = utils.config['sglversion_pkgname']
    cache_dir = utils.tdnf_config.get('main', 'cachedir')

    before = utils.run(['find', cache_dir, '-name', pkgname + '*.rpm'])
    ret = utils.run(['tdnf', '--dryrun', 'install', pkgname])
    assert ret['retval'] == 0
    after = utils.run(['find', cache_dir, '-name', pkgname + '*.rpm'])

    assert before['stdout'] == after['stdout']
    assert not utils.check_package(pkgname)


# dry-run with a package that has dependencies: deps must appear in plan
def test_dry_run_install_with_deps(utils):
    pkgname = 'tdnf-test-cleanreq-leaf1'
    dep_pkgname = 'tdnf-test-cleanreq-required'

    leaf_was_installed = utils.check_package(pkgname)
    dep_was_installed = utils.check_package(dep_pkgname)

    utils.erase_package(pkgname)
    utils.erase_package(dep_pkgname)

    ret = utils.run(['tdnf', '--dryrun', 'install', pkgname])
    assert ret['retval'] == 0
    assert not utils.check_package(pkgname)
    assert not utils.check_package(dep_pkgname)
    output = '\n'.join(ret['stdout'] + ret['stderr'])
    assert pkgname in output

    if dep_was_installed:
        utils.install_package(dep_pkgname)
    if leaf_was_installed:
        utils.install_package(pkgname)
