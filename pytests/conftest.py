#            (c) 2019 VMware Inc.,
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
    def __init__(self):
        self.config = JsonWrapper('config.json').read()

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

@pytest.fixture(scope='session')
def utils():
    test_utils = TestUtils()
    backup_config_files()
    yield test_utils
    restore_config_files()
