#
# Copyright (C) 2025 Broadcom, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.

import base64
import os
import pytest

from http.server import SimpleHTTPRequestHandler, HTTPServer
from multiprocessing import Process


HTTP_PORT = 8088

USERNAME = "cassian"
PASSWORD = "andor"
AUTH_STRING = "Basic " + base64.b64encode(f"{USERNAME}:{PASSWORD}".encode()).decode()


class AuthHandler(SimpleHTTPRequestHandler):
    def do_GET(self):
        if self.headers.get("Authorization") == AUTH_STRING:
            return super().do_GET()
        else:
            self.send_response(401)
            self.send_header("WWW-Authenticate", "Basic realm='Test'")
            self.send_header("Content-type", "text/html")
            self.end_headers()
            self.wfile.write(b"Authentication required")


def AuthRepoServer(root, port=HTTP_PORT, interface=''):
    addr = (interface, port)

    os.chdir(root)

    httpd = HTTPServer(addr, AuthHandler)
    httpd.serve_forever()


@pytest.fixture(scope='module', autouse=True)
def start_server(utils):
    server = Process(target=AuthRepoServer,
                     args=(utils.config['repo_path'], ))
    server.start()

    yield

    server.terminate()
    server.join()


@pytest.fixture(scope='function', autouse=True)
def setup_test_function(utils):
    pkgname = utils.config["sglversion_pkgname"]
    utils.erase_package(pkgname)
    yield
    utils.erase_package(pkgname)


def test_install_package(utils):
    pkgname = utils.config["sglversion_pkgname"]

    ret = utils.run(['tdnf', 'install', '--repoid=photon-test-auth', '-y', '--nogpgcheck', pkgname])
    assert ret['retval'] == 0
    assert utils.check_package(pkgname)
