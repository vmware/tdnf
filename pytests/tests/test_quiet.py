#
# Copyright (C) 2019 - 2023 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import pytest


@pytest.fixture(scope="module", autouse=True)
def setup_test(utils):
    yield
    pkg = utils.config["sglversion_pkgname"]
    teardown_test(utils, pkg)


def teardown_test(utils, pkg):
    cmd = f"tdnf erase -qy {pkg}".split()
    utils.run(cmd)


def ensure_empty_ret(ret):
    assert ret["retval"] == 0
    assert not "\n".join(ret["stdout"])
    stderr_l = []
    for ele in ret["stderr"]:
        if "NOKEY" not in ele:
            stderr_l.append(ele)

    assert not stderr_l


err_str = "--assumeyes|-y or --assumeno is required when quiet is set"


def ensure_non_empty_ret(ret):
    assert ret["retval"] == 918
    assert not "\n".join(ret["stdout"])
    assert err_str in "\n".join(ret["stderr"])


def test_quiet_install(utils):
    pkg = utils.config["sglversion_pkgname"]

    for opt in ["-qy", "-q -y", "--quiet --assumeyes"]:
        teardown_test(utils, pkg)
        cmd = f"tdnf install {opt} --nogpgcheck {pkg}".split()
        ret = utils.run(cmd)
        ensure_empty_ret(ret)


def test_quiet_erase(utils):
    pkg = utils.config["sglversion_pkgname"]

    for opt in ["-qy", "-q -y", "--quiet --assumeyes"]:
        cmd = f"tdnf install -y --nogpgcheck {pkg}".split()
        utils.run(cmd)
        cmd = f"tdnf erase {opt} {pkg}".split()
        ret = utils.run(cmd)
        ensure_empty_ret(ret)


def test_quiet_negative(utils):
    name = utils.config["mulversion_pkgname"]
    ver = utils.config["mulversion_lower"]

    teardown_test(utils, f"{name}")

    cmd = f"tdnf install -q {name}-{ver}".split()
    ret = utils.run(cmd)
    ensure_non_empty_ret(ret)

    cmd = f"tdnf install -qy {name}-{ver}".split()
    utils.run(cmd)

    for opt in ["erase", "update"]:
        cmd = f"tdnf {opt} -q {name}".split()
        ret = utils.run(cmd)
        ensure_non_empty_ret(ret)

    teardown_test(utils, f"{name}")


def test_quiet_makecache_clean(utils):
    cmd = "tdnf -q clean all".split()
    ret = utils.run(cmd)
    ensure_empty_ret(ret)

    cmd = "tdnf makecache --refresh -q".split()
    ret = utils.run(cmd)
    assert ret["retval"] == 0
    assert "Metadata cache created." "\n".join(ret["stdout"])
    assert not "\n".join(ret["stderr"])
