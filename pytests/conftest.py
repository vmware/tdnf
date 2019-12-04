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
import re
import errno
import shutil
import ssl
import requests
from pprint import pprint
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
        if cli_args:
            self.config.update(cli_args)

    def assert_file_exists(self, file_path):
        if not os.path.isfile(file_path):
            raise FileNotFoundError(errno.ENOENT, os.strerror(errno.ENOENT), file_path)

    def enable_repo(self, repo):
        repo_path = os.path.join('/etc/yum.repos.d/', repo)
        self.assert_file_exists(repo_path)
        self.run([ 'sed', '-i', '/enabled=/d', repo_path ])
        self.run([ 'sed', '-i', '$ a enabled=1', repo_path ])

    def disable_repo(self, repo):
        repo_path = os.path.join('/etc/yum.repos.d/', repo)
        self.assert_file_exists(repo_path)
        self.run([ 'sed', '-i', '/enabled=/d', repo_path ])
        self.run([ 'sed', '-i', '$ a enabled=0', repo_path ])

    def check_package(self, package):
        """ Check if a package exists """
        ret = self.run([ 'tdnf', 'list', package ])
        for line in ret['stdout']:
            if package in line and '@System' in line:
                return True
        return False

    def erase_package(self, pkgname, pkgversion=None):
        if pkgversion:
            pkg = pkgname + '-' + pkgversion
        else:
            pkg = pkgname
        self.run([ 'tdnf', 'erase', '-y', pkg ])
        assert(self.check_package(pkgname) == False)

    def install_package(utils, pkgname, pkgversion=None):
        if pkgversion:
            pkg = pkgname + '-' + pkgversion
        else:
            pkg = pkgname
        utils.run([ 'tdnf', 'install', '-y', '--nogpgcheck', pkg ])
        assert(utils.check_package(pkgname) == True)

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
        if cmd[0] is 'tdnf' and 'build_dir' in self.config and self.config['build_dir']:
            cmd[0] = os.path.join(self.config['build_dir'], 'bin/tdnf')
        use_shell = not isinstance(cmd, list)
        process = subprocess.Popen(cmd, shell=use_shell,
                                   stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE)
        #process.wait()
        out, err = process.communicate()
        stdout = out.decode().strip()
        stderr = err.decode().strip()
        retval = process.returncode
        capture = re.match(r'^Error\((\d+)\) :', stderr)
        if capture:
            retval = int(capture.groups()[0])

        ret = {}
        if stdout:
            ret['stdout'] = stdout.split('\n')
        else:
            ret['stdout'] = []
        if stderr:
            ret['stderr'] = stderr.split('\n')
        else:
            ret['stderr'] = []
        ret['retval'] = retval
        return ret

def backup_files(conf_dir):
    for file in os.listdir(conf_dir):
        if file.endswith('.bak'):
            continue
        src  = os.path.join(conf_dir, file)
        dest = os.path.join(conf_dir, os.path.basename(file) + '.bak')
        shutil.copyfile(src, dest)

def restore_files(conf_dir):
    for file in os.listdir(conf_dir):
        if not file.endswith('.bak'):
            continue
        src = os.path.join(conf_dir, file)
        dest = os.path.join(conf_dir, os.path.basename(file).replace('.bak', ''))
        shutil.move(src, dest)

def backup_config_files(utils):
    # Backup /etc/yum.repos.d/* and /etc/tdnf/tdnf.conf
    backup_files('/etc/yum.repos.d/')
    backup_files('/etc/tdnf/')
    utils.assert_file_exists('/etc/yum.repos.d/photon-test.repo')

def restore_config_files(utils):
    # Restore /etc/yum.repos.d/* and /etc/tdnf/tdnf.conf
    restore_files('/etc/yum.repos.d/')
    restore_files('/etc/tdnf/')

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
    backup_config_files(test_utils)
    yield test_utils
    restore_config_files(test_utils)
