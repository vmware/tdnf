#
# Copyright (C) 2026 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import pytest

PREIN_BASE = "tdnf-test-prein-base"
PREIN_DEP_A = "tdnf-test-prein-dep-a"
PREIN_DEP_B = "tdnf-test-prein-dep-b"
PREIN_CONSUMER = "tdnf-test-prein-consumer"
PREIN_PKGS = [PREIN_BASE, PREIN_DEP_A, PREIN_DEP_B, PREIN_CONSUMER]


@pytest.fixture(scope="function", autouse=True)
def cleanup_prein_packages(utils):
    utils.run(["tdnf", "remove", "-y"] + PREIN_PKGS)
    yield
    utils.run(["tdnf", "remove", "-y"] + PREIN_PKGS)


def _install_and_get_order(utils, pkgs):
    ret = utils.run(["tdnf", "install", "-y", "--nogpgcheck"] + pkgs)
    assert ret["retval"] == 0, "tdnf install failed (retval={}):\n{}".format(
        ret["retval"], "\n".join(ret["stdout"] + ret["stderr"])
    )
    return [line for line in ret["stdout"] if line.startswith("Installing/Updating:")]


def _remove_prein_packages(utils):
    ret = utils.run(["tdnf", "remove", "-y"] + PREIN_PKGS)
    assert ret["retval"] == 0, "tdnf remove failed (retval={}):\n{}".format(
        ret["retval"], "\n".join(ret["stdout"] + ret["stderr"])
    )


# Regression test for transaction_order() fix in client/goal.c:
# without the fix, solver_create_transaction() leaves pTrans->steps in
# CLI-argument order; rpmtsOrder() then picks whichever Requires(pre)
# provider appears closest to the consumer as the ordering anchor, making
# the install sequence depend on CLI argument order.
def test_prein_install_order_is_deterministic(utils):
    order_a_first = _install_and_get_order(
        utils, [PREIN_DEP_A, PREIN_DEP_B, PREIN_CONSUMER]
    )

    _remove_prein_packages(utils)

    order_b_first = _install_and_get_order(
        utils, [PREIN_DEP_B, PREIN_DEP_A, PREIN_CONSUMER]
    )

    assert order_a_first == order_b_first, (
        "Install order differed with different CLI argument order -- "
        "transaction_order() may not be called after solver_create_transaction().\n"
        "dep-a first: {}\ndep-b first: {}".format(order_a_first, order_b_first)
    )
