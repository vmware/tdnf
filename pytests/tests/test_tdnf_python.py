#!/usr/bin/env python3
#
# Copyright (C) 2019-2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import os
import sys
import pytest


@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    global mulpkgname
    mulpkgname = utils.config['mulversion_pkgname']

    global sglpkgname
    sglpkgname = utils.config['sglversion_pkgname']

    global config
    config = os.path.join(utils.config['repo_path'], "tdnf.conf")

    if not utils.config.get('installed', False):
        # find path to our tdnf python module
        builddir = utils.config['build_dir']
        arch = os.uname().machine
        pymajor = sys.version_info.major
        pyminor = sys.version_info.minor

        # only support python3
        assert pymajor == 3

        if pyminor < 11:
            path = f"{builddir}/python/build/lib.linux-{arch}-{pymajor}.{pyminor}"
        else:
            path = f"{builddir}/python/build/lib.linux-{arch}-cpython-{pymajor}{pyminor}"

        assert os.path.isdir(path)
        sys.path.append(path)

    global tdnf
    import tdnf

    yield
    teardown_test(utils)


def teardown_test(utils):
    utils.run(['tdnf', 'erase', '-y', sglpkgname])
    utils.run(['tdnf', 'erase', '-y', mulpkgname])
    assert not utils.check_package(sglpkgname)
    assert not utils.check_package(mulpkgname)


def tdnf_py_erase(pkgs, quiet=False, refresh=False, cfg=None):
    conf = cfg if cfg else config
    pkgs = [x for x in pkgs if x]
    for p in pkgs:
        try:
            tdnf.erase(pkgs=[p], quiet=quiet, refresh=refresh, config=conf)
        except Exception:
            pass


def tdnf_py_install(pkgs, quiet=False, refresh=False, cfg=None):
    pkgs = [x for x in pkgs if x]
    conf = cfg if cfg else config
    tdnf.install(pkgs=pkgs, quiet=quiet, refresh=refresh, config=conf)


def tdnf_py_repolist(filter=None, cfg=None):
    conf = cfg if cfg else config
    if filter:
        return tdnf.repolist(filter=filter, config=conf)

    return tdnf.repolist(config=conf)


def tdnf_py_distro_sync(quiet=False, refresh=False, cfg=None):
    conf = cfg if cfg else config
    tdnf.distro_sync(quiet=quiet, refresh=refresh, config=conf)


def tdnf_py_downgrade(pkgs, quiet=False, refresh=False, cfg=None):
    pkgs = [x for x in pkgs if x]
    conf = cfg if cfg else config
    tdnf.downgrade(pkgs=pkgs, quiet=quiet, refresh=refresh, config=conf)


def tdnf_py_update(pkgs=[], quiet=False, refresh=False, cfg=None):
    pkgs = [x for x in pkgs if x]
    conf = cfg if cfg else config
    if pkgs:
        tdnf.update(pkgs=pkgs, quiet=quiet, refresh=refresh, config=conf)
    else:
        tdnf.update(quiet=quiet, refresh=refresh, config=conf)


def test_tdnf_python_install_single(utils):
    pkgs = [sglpkgname]

    tdnf_py_erase(pkgs=pkgs)
    assert not utils.check_package(sglpkgname)
    tdnf_py_install(pkgs=pkgs)
    assert utils.check_package(sglpkgname)

    tdnf_py_erase(pkgs=pkgs)
    assert not utils.check_package(sglpkgname)
    tdnf_py_install(pkgs=pkgs, refresh=True)
    assert utils.check_package(sglpkgname)

    tdnf_py_erase(pkgs=pkgs)
    assert not utils.check_package(sglpkgname)
    tdnf_py_install(pkgs=pkgs, refresh=True, quiet=True)
    assert utils.check_package(sglpkgname)


def test_tdnf_python_install_multiple(utils):
    pkgs = [sglpkgname, mulpkgname]

    tdnf_py_erase(pkgs=pkgs)
    assert not utils.check_package(sglpkgname)
    assert not utils.check_package(mulpkgname)
    tdnf_py_install(pkgs=pkgs)
    assert utils.check_package(sglpkgname)
    assert utils.check_package(mulpkgname)

    tdnf_py_erase(pkgs=pkgs)
    assert not utils.check_package(sglpkgname)
    assert not utils.check_package(mulpkgname)
    tdnf_py_install(pkgs=pkgs, refresh=True)
    assert utils.check_package(sglpkgname)
    assert utils.check_package(mulpkgname)

    tdnf_py_erase(pkgs=pkgs)
    assert not utils.check_package(sglpkgname)
    assert not utils.check_package(mulpkgname)
    tdnf_py_install(pkgs=pkgs, refresh=True, quiet=True)
    assert utils.check_package(sglpkgname)
    assert utils.check_package(mulpkgname)


def test_tdnf_python_erase_single(utils):
    pkgs = [sglpkgname]

    tdnf_py_install(pkgs=pkgs)
    assert utils.check_package(sglpkgname)
    tdnf_py_erase(pkgs=pkgs)
    assert not utils.check_package(sglpkgname)


def test_tdnf_python_erase_multiple(utils):
    pkgs = [sglpkgname, mulpkgname]

    tdnf_py_install(pkgs=pkgs)
    assert utils.check_package(sglpkgname)
    assert utils.check_package(mulpkgname)
    tdnf_py_erase(pkgs=pkgs)
    assert not utils.check_package(sglpkgname)
    assert not utils.check_package(mulpkgname)


def check_repodata(data, filter=0):
    if filter != 2:
        assert data.enabled == 1
        assert data.name.decode('utf-8') == 'basic'
        assert data.id.decode('utf-8') == 'photon-test'
        assert data.baseurl.decode('utf-8') == 'http://localhost:8080/photon-test'
    else:
        assert data.enabled == 0
        assert data.name.decode('utf-8') == '@cmdline'
        assert data.id.decode('utf-8') == '@cmdline'
        assert not hasattr(data, 'baseurl')


def test_tdnf_python_repolist(utils):
    check_repodata(tdnf_py_repolist()[0])

    for i in [0, 1, 2, -1, 10]:
        check_repodata(tdnf_py_repolist(i)[0], i)


def test_tdnf_python_update_single(utils):
    pkgs = [sglpkgname]
    tdnf_py_install(pkgs=pkgs)
    tdnf_py_update(pkgs=pkgs)
    tdnf_py_update(pkgs=pkgs, quiet=True)
    tdnf_py_update(pkgs=pkgs, quiet=True, refresh=True)


def test_tdnf_python_update_multiple(utils):
    pkgs = [sglpkgname, mulpkgname + '-' + utils.config['mulversion_lower']]

    tdnf_py_erase(pkgs=[sglpkgname, mulpkgname])
    tdnf_py_install(pkgs=pkgs)
    tdnf_py_update(pkgs=pkgs)

    tdnf_py_erase(pkgs=[sglpkgname, mulpkgname])
    tdnf_py_install(pkgs=pkgs)
    tdnf_py_update(pkgs=pkgs, quiet=True)

    tdnf_py_erase(pkgs=[sglpkgname, mulpkgname])
    tdnf_py_install(pkgs=pkgs)
    tdnf_py_update(pkgs=pkgs, quiet=True, refresh=True)


def test_tdnf_python_update_all(utils):
    tdnf_py_update()


def test_tdnf_python_downgrade_single(utils):
    pkgs = [mulpkgname]
    tdnf_py_erase(pkgs=pkgs)
    tdnf_py_install(pkgs=pkgs)
    tdnf_py_downgrade(pkgs=pkgs, refresh=True)
    tdnf_py_downgrade(pkgs=pkgs, quiet=True, refresh=True)


def test_tdnf_python_downgrade_multiple(utils):
    pkgs = [sglpkgname, mulpkgname]
    tdnf_py_erase(pkgs=pkgs)
    tdnf_py_install(pkgs=pkgs)
    tdnf_py_downgrade(pkgs=pkgs, refresh=True)
    tdnf_py_downgrade(pkgs=pkgs, quiet=True, refresh=True)


def test_tdnf_python_distro_sync(utils):
    tdnf_py_distro_sync(quiet=True)
    tdnf_py_distro_sync(refresh=True, quiet=True)


def test_tdnf_python_repofilter(utils):
    assert tdnf.REPOLISTFILTER_ALL == 0
    assert tdnf.REPOLISTFILTER_ENABLED == 1
    assert tdnf.REPOLISTFILTER_DISABLED == 2
