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
import configparser
import distutils.spawn
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
        script = os.path.join(self.config['test_path'], 'repo/setup-repo.sh')
        self.run([ 'sh', script, self.config['repo_path'] ])
        self.tdnf_config = configparser.ConfigParser()
        self.tdnf_config.read(os.path.join(self.config['repo_path'], 'tdnf.conf'))
        self.config['valgrind_enabled'] = False
        #check execution environment and enable valgrind if suitable
        self.check_valgrind()

    def _version_number(self, version):
        version_parts = version.split('.')
        return version_parts[0] * 1000 + version_parts[1] * 100 + version_parts[2]

    #valgrind versions less than minimum required might not
    #support all the options specified or might have known
    #errors
    def check_valgrind(self):
        valgrind = distutils.spawn.find_executable('valgrind')
        if not valgrind:
            self.config['valgrind_disabled_reason'] = 'valgrind not found'
        else:
            stream = os.popen(valgrind + ' --version') #nosec
            valgrind_version = stream.read()
            if valgrind_version:
                valgrind_version = valgrind_version.replace('valgrind-', '')
                valgrind_minimum_version = self.config['valgrind_minimum_version']
                if self._version_number(valgrind_version) >= \
                   self._version_number(valgrind_minimum_version):
                    self.config['valgrind_enabled'] = True
                else:
                    self.config['valgrind_disabled_reason'] = \
                    'valgrind minimum version required: ' + valgrind_minimum_version

    def assert_file_exists(self, file_path):
        if not os.path.isfile(file_path):
            raise FileNotFoundError(errno.ENOENT, os.strerror(errno.ENOENT), file_path)

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

    def _decorate_tdnf_cmd_for_test(self, cmd):
        if cmd[0] is 'tdnf':
            if 'build_dir' in self.config:
                cmd[0] = os.path.join(self.config['build_dir'], 'bin/tdnf')
            if cmd[1] is not '--config':
                cmd.insert(1, '-c')
                cmd.insert(2, os.path.join(self.config['repo_path'], 'tdnf.conf'))

    def _skip_if_valgrind_disabled(self):
        if  not self.config['valgrind_enabled']:
            pytest.skip(self.config['valgrind_disabled_reason'])

    def run_memcheck(self, cmd):
        self._skip_if_valgrind_disabled()
        self._decorate_tdnf_cmd_for_test(cmd)
        memcheck_cmd = ['valgrind',
                        '--leak-check=full',
                        '--exit-on-first-error=yes',
                        '--error-exitcode=1']
        return self._run(memcheck_cmd + cmd, retvalonly=True)

    def run(self, cmd):
        self._decorate_tdnf_cmd_for_test(cmd)
        return self._run(cmd)

    def _run(self, cmd, retvalonly=False):
        use_shell = not isinstance(cmd, list)
        process = subprocess.Popen(cmd, shell=use_shell, #nosec
                                   stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE)
        #process.wait()
        out, err = process.communicate()
        if retvalonly:
            return {'retval':process.returncode}

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

def pytest_addoption(parser):
    group = parser.getgroup("tdnf", "tdnf specifc options")
    group.addoption(
        "--build-dir", action="store", default="",
        help="directory containing tdnf binary to be tested"
    )

@pytest.fixture(scope='session')
def tdnf_args(request):
    arg = {}
    build_dir = request.config.getoption("--build-dir")
    if build_dir:
        arg['build_dir'] = build_dir
    return arg

@pytest.fixture(scope='session')
def utils(tdnf_args):
    test_utils = TestUtils(tdnf_args)
    return test_utils
