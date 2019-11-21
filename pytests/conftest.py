#
# Copyright (C) 2019 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#
#   Author: Siddharth Chandrasekaran <csiddharth@vmware.com>

import pytest
import json
import subprocess
import os
import shutil
import ssl
import requests
from urllib.parse import urlparse
from OpenSSL.crypto import load_certificate, FILETYPE_PEM

class JsonWrapper(object):

    def __init__(self, filename):
        self.filename = filename

    def read(self):
        with open(self.filename) as json_data:
            self.data = json.load(json_data)
        return self.data

    def write(self, data):
        self.data = data
        with open(self.filename, 'wb') as outfile:
            json.dump(data, outfile)


class TestUtils(object):
    def __init__(self, cli_args):
        cur_dir = os.path.dirname(os.path.realpath(__file__))
        config_file = os.path.join(cur_dir, 'config.json')
        self.config = JsonWrapper(config_file).read()
        self.config.update(cli_args)

    def check_package(self, package):
        """ Check if a package exists """
        ret = self.run([ 'tdnf', 'list', package ])
        for line in ret['stdout']:
            if package in line and '@System' in line:
                return True
        return False

    def remove_package(self, package):
        ret = self.run([ 'tdnf', 'erase', '-y', package ])
        if ret['retval'] != 0:
            return False
        return True

    def _requests_get(self, url, verify):
        try:
            r = requests.get(url, verify=verify, stream=True, timeout=5.0)
        except:
            return None
        return r

    def wget(self, url, out, enforce_https=True, fingerprint=None):
        # Check URL
        try:
            u = urlparse(url)
        except:
            return False, "Failed to parse URL"
        if not all([ u.scheme, u.netloc ]):
            return False, 'Invalid URL'
        if enforce_https:
            if u.scheme != 'https':
                return False, 'URL must be of secure origin (HTTPS)'
        r = self._requests_get(url, True)
        if r is None:
            if fingerprint is None:
                return False, "Unable to verify server certificate"
            port = u.port
            if port is None:
                port = 443
            try:
                pem = ssl.get_server_certificate((u.netloc, port))
                cert = load_certificate(FILETYPE_PEM, pem)
                fp = cert.digest('sha1').decode()
            except:
                return False, "Failed to get server certificate"
            if fingerprint != fp:
                return False, "Server fingerprint did not match provided. Got: " + fp
            # Download file without validation
            r = self._requests_get(url, False)
            if r is None:
                return False, "Failed to download file"
        r.raw.decode_content = True
        with open(out, 'wb') as f:
            shutil.copyfileobj(r.raw, f)

        return True, None

    def run(self, cmd):
        if cmd[0] is 'tdnf' and self.config['build_dir']:
            cmd[0] = os.path.join(self.config['build_dir'], 'bin/tdnf')
        use_shell = not isinstance(cmd, list)
        process = subprocess.Popen(cmd, shell=use_shell,
                                   stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE)
        process.wait()
        out, err = process.communicate()
        ret = {}
        ret['stdout'] = []
        ret['stdout'] = []
        ret['retval'] = process.returncode
        if out:
            ret['stdout'] = out.decode().split('\n')
        if err:
            ret['strerr'] = err.decode().split('\n')

        return ret

def backup_config_files():
    # Backup /etc/yum.repos.d/* and /etc/tdnf/tdnf.conf
    pass

def restore_config_files():
    # Restore /etc/yum.repos.d/* and /etc/tdnf/tdnf.conf
    pass

def pytest_addoption(parser):
    group = parser.getgroup("tdnf", "tdnf specifc options")
    group.addoption(
        "--build-dir", action="store", default="",
        help="directory containing tdnf binary to be tested"
    )

@pytest.fixture(scope='session')
def tdnf_args(request):
    arg = {}
    arg['build_dir'] = request.config.getoption("--build-dir")
    return arg

@pytest.fixture(scope='session')
def utils(tdnf_args):
    test_utils = TestUtils(tdnf_args)
    backup_config_files()
    yield test_utils
    restore_config_files()
