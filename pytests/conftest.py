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
from OpenSSL import crypto, SSL
from http.server import SimpleHTTPRequestHandler, HTTPServer
import socketserver
import threading
import ssl
import platform

ARCH=platform.machine()

class TestRepoServer(threading.Thread):

    def __init__(self, root, port=8080, interface="", enable_https=False):
        super().__init__()
        self.daemon = True
        self.port = port
        self.root = root
        self.addr = (interface, port)
        self.enable_https = enable_https

    def make_cert(self):
        if os.path.exists("cert.pem") and os.path.exists("key.pem"):
            self.keyfile = os.path.join(os.getcwd(), 'key.pem')
            self.certfile = os.path.join(os.getcwd(), 'cert.pem')
            return
        k = crypto.PKey()
        k.generate_key(crypto.TYPE_RSA, 2048)
        cert = crypto.X509()
        cert.get_subject().C = "IN"
        cert.get_subject().ST = "Karnataka"
        cert.get_subject().L = "Bangalore"
        cert.get_subject().O = "VMware"
        cert.get_subject().OU = "Photon OS"
        cert.get_subject().CN = "pytest.tdnf.vmware.github.io"
        cert.get_subject().emailAddress = "tdnf-devel@vmware.com"
        cert.set_serial_number(0)
        cert.gmtime_adj_notBefore(0)
        cert.gmtime_adj_notAfter(365*24*60*60)
        cert.set_issuer(cert.get_subject())
        cert.set_pubkey(k)
        cert.sign(k, "sha512")
        with open("cert.pem", "wt") as f:
            f.write(crypto.dump_certificate(crypto.FILETYPE_PEM, cert).decode("utf-8"))
        with open("key.pem", "wt") as f:
            f.write(crypto.dump_privatekey(crypto.FILETYPE_PEM, k).decode("utf-8"))
        self.keyfile = os.path.join(os.getcwd(), 'key.pem')
        self.certfile = os.path.join(os.getcwd(), 'cert.pem')

    def run(self):
        if self.enable_https:
            self.make_cert()
        os.chdir(self.root)
        try:
            self.httpd = HTTPServer(self.addr, SimpleHTTPRequestHandler)
            if self.enable_https:
                self.httpd.socket = ssl.wrap_socket(self.httpd.socket,
                                                    keyfile = self.keyfile,
                                                    certfile = self.certfile,
                                                    server_side = True)
            self.httpd.serve_forever()
        finally:
            self.httpd.server_close()

    def join(self):
        self.httpd.shutdown()
        super().join()


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

    def __init__(self, cli_args=None):
        cur_dir = os.path.dirname(os.path.realpath(__file__))
        config_file = os.path.join(cur_dir, 'config.json')
        self.config = JsonWrapper(config_file).read()
        if cli_args:
            self.config.update(cli_args)
        self.config['distribution'] = os.environ.get('DIST', 'photon')
        script = os.path.join(self.config['test_path'], 'repo/setup-repo.sh')
        self.run([ 'sh', script, self.config['repo_path'] ])
        self.tdnf_config = configparser.ConfigParser()
        self.tdnf_config.read(os.path.join(self.config['repo_path'], 'tdnf.conf'))

        # check execution environment and enable valgrind if suitable
        self.check_valgrind()

        self.server = TestRepoServer(self.config['repo_path'])
        self.server.start()

    def version_str_to_int(self, version):
        version_parts = version.split('.')
        return version_parts[0] * 1000 + version_parts[1] * 100 + version_parts[2]

    def check_valgrind(self):
        self.config['valgrind_enabled'] = False
        valgrind = distutils.spawn.find_executable('valgrind')
        if not valgrind:
            self.config['valgrind_disabled_reason'] = 'valgrind not found'
            return

        stream = os.popen(valgrind + ' --version') #nosec
        valgrind_version = stream.read()
        if not valgrind_version:
            self.config['valgrind_disabled_reason'] = 'Unable to ascertain valgrind version'
            return

        # valgrind versions less than minimum required might not support
        # all the options specified or might have known errors.
        current_version = self.version_str_to_int(valgrind_version.replace('valgrind-', ''))
        minimum_version = self.version_str_to_int(self.config['valgrind_minimum_version'])
        if current_version < minimum_version:
            self.config['valgrind_disabled_reason'] = 'valgrind minimum version constraint not met'
            return

        # some architectures may require a higher version
        if 'valgrind_minimum_version_{}'.format(ARCH) in self.config:
            minimum_version = \
                self.version_str_to_int(self.config['valgrind_minimum_version_{}'.format(ARCH)])
            if current_version < minimum_version:
                self.config['valgrind_disabled_reason'] = \
                    'valgrind minimum version constraint not met for this architecture'
                return

        # All okay. Enable valgrind checks
        self.config['valgrind_disabled_reason'] = None
        self.config['valgrind_enabled'] = True

    def assert_file_exists(self, file_path):
        if not os.path.isfile(file_path):
            raise FileNotFoundError(errno.ENOENT, os.strerror(errno.ENOENT), file_path)

    def check_package(self, package, version=None):
        """ Check if a package exists """
        ret = self.run([ 'tdnf', 'list', package ])
        for line in ret['stdout']:
            if package in line and '@System' in line:
                if version == None or version in line:
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
        if cmd[0] == 'tdnf':
            if 'build_dir' in self.config:
                cmd[0] = os.path.join(self.config['build_dir'], 'bin/tdnf')
            if ('-c' not in cmd and '--config' not in cmd):
                cmd.insert(1, '-c')
                cmd.insert(2, os.path.join(self.config['repo_path'], 'tdnf.conf'))

    def _skip_if_valgrind_disabled(self):
        if not self.config['valgrind_enabled']:
            pytest.skip(self.config['valgrind_disabled_reason'])

    def run_memcheck(self, cmd):
        self._skip_if_valgrind_disabled()
        self._decorate_tdnf_cmd_for_test(cmd)
        memcheck_cmd = ['valgrind',
                        '--leak-check=full',
                        '--exit-on-first-error=yes',
                        '--error-exitcode=1']
        return self._run(memcheck_cmd + cmd, retvalonly=True)

    def run(self, cmd, cwd=None):
        self._decorate_tdnf_cmd_for_test(cmd)
        return self._run(cmd, cwd=cwd)

    def _run(self, cmd, retvalonly=False, cwd=None):
        use_shell = not isinstance(cmd, list)
        print(cmd)
        process = subprocess.Popen(cmd, shell=use_shell, #nosec
                                   stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE,
                                   cwd=cwd)
        #process.wait()
        out, err = process.communicate()
        if retvalonly:
            return {'retval':process.returncode}

        stdout = out.decode().strip()
        stderr = err.decode().strip()
        retval = process.returncode
        capture = re.search(r'^Error\((\d+)\) :', stderr, re.MULTILINE)
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


@pytest.fixture(scope='session')
def utils():
    test_utils = TestUtils()
    return test_utils
