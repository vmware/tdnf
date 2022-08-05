#
# Copyright (C) 2019-2022 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

import os
import re
import ssl
import sys
import time
import json
import errno
import socket
import shutil
import pytest
import atexit
import platform
import requests
import subprocess
import configparser
from OpenSSL import crypto
from urllib.parse import urlparse
from multiprocessing import Process
from http.server import SimpleHTTPRequestHandler, HTTPServer

ARCH = platform.machine()


def StopTestRepoServer(server):
    server.terminate()
    server.join()


def TestRepoServer(root, port=8080, interface='', enable_https=False):
    addr = (interface, port)
    enable_https = enable_https

    def dump_cert(fn, cert):
        with open(fn, 'wt') as f:
            tmp = crypto.dump_certificate(crypto.FILETYPE_PEM, cert)
            f.write(tmp.decode('utf-8'))

    os.chdir(root)

    if enable_https:
        k = crypto.PKey()
        k.generate_key(crypto.TYPE_RSA, 2048)
        cert = crypto.X509()
        cert.get_subject().C = 'IN'
        cert.get_subject().ST = 'Karnataka'
        cert.get_subject().L = 'Bangalore'
        cert.get_subject().O = 'VMware'  # noqa: E741
        cert.get_subject().OU = 'Photon OS'
        cert.get_subject().CN = 'pytest.tdnf.vmware.github.io'
        cert.get_subject().emailAddress = 'tdnf-devel@vmware.com'
        cert.set_serial_number(0)
        cert.gmtime_adj_notBefore(0)
        cert.gmtime_adj_notAfter(365 * 24 * 60 * 60)
        cert.set_issuer(cert.get_subject())
        cert.set_pubkey(k)
        cert.sign(k, 'sha512')

        keyfile = os.path.join(os.getcwd(), 'key.pem')
        dump_cert(keyfile, cert)

        certfile = os.path.join(os.getcwd(), 'cert.pem')
        dump_cert(keyfile, k)

    httpd = HTTPServer(addr, SimpleHTTPRequestHandler)
    if enable_https:
        httpd.socket = ssl.wrap_socket(httpd.socket, keyfile=keyfile,
                                       certfile=certfile, server_side=True)
    httpd.serve_forever()


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
        ret = self.run(['sh', script, self.config['repo_path']])
        if ret['retval']:
            pytest.exit("An error occured while running {}, stdout: \n{}".format(
                script,
                "\n".join(ret['stdout'])
            ))
        self.tdnf_config = configparser.ConfigParser()
        self.tdnf_config.read(os.path.join(self.config['repo_path'],
                                           'tdnf.conf'))

        # check execution environment and enable valgrind if suitable
        self.check_valgrind()

    def version_str_to_int(self, version):
        version_parts = version.split('.')
        return version_parts[0] * 1000 + version_parts[1] * 100 + version_parts[2]

    def check_valgrind(self):
        self.config['valgrind_enabled'] = False
        valgrind = shutil.which('valgrind')
        if not valgrind:
            self.config['valgrind_disabled_reason'] = 'valgrind not found'
            return

        stream = os.popen(valgrind + ' --version')  # nosec
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
            raise FileNotFoundError(errno.ENOENT, os.strerror(errno.ENOENT),
                                    file_path)

    def check_package(self, package, version=None):
        ''' Check if a package exists '''
        ret = self.run(['tdnf', 'list', package])
        for line in ret['stdout']:
            if package in line and '@System' in line:
                if version is None or version in line:
                    return True
        return False

    def erase_package(self, pkgname, pkgversion=None):
        if pkgversion:
            pkg = pkgname + '-' + pkgversion
        else:
            pkg = pkgname
        self.run(['tdnf', 'erase', '-y', pkg])
        assert not self.check_package(pkgname)

    def install_package(utils, pkgname, pkgversion=None):
        if pkgversion:
            pkg = pkgname + '-' + pkgversion
        else:
            pkg = pkgname
        utils.run(['tdnf', 'install', '-y', '--nogpgcheck', pkg])
        assert utils.check_package(pkgname)

    def _requests_get(self, url, verify):
        try:
            r = requests.get(url, verify=verify, stream=True, timeout=5.0)
        except Exception:
            return None
        return r

    def wget(self, url, out, enforce_https=True, fingerprint=None):
        # Check URL
        try:
            u = urlparse(url)
        except Exception:
            return False, 'Failed to parse URL'
        if not all([u.scheme, u.netloc]):
            return False, 'Invalid URL'
        if enforce_https:
            if u.scheme != 'https':
                return False, 'URL must be of secure origin (HTTPS)'
        r = self._requests_get(url, True)
        if r is None:
            if fingerprint is None:
                return False, 'Unable to verify server certificate'
            port = u.port
            if port is None:
                port = 443
            try:
                pem = ssl.get_server_certificate((u.netloc, port))
                cert = ssl.load_certificate(crypto.FILETYPE_PEM, pem)
                fp = cert.digest('sha1').decode()
            except Exception:
                return False, 'Failed to get server certificate'
            if fingerprint != fp:
                return False, 'Server fingerprint did not match provided. Got: ' + fp
            # Download file without validation
            r = self._requests_get(url, False)
            if r is None:
                return False, 'Failed to download file'
        r.raw.decode_content = True
        with open(out, 'wb') as f:
            shutil.copyfileobj(r.raw, f)
        return True, None

    def _decorate_tdnf_cmd_for_test(self, cmd, noconfig=False):
        if cmd[0] == 'tdnf':
            if 'build_dir' in self.config:
                cmd[0] = os.path.join(self.config['build_dir'], 'bin/tdnf')
            if ('-c' not in cmd and '--config' not in cmd and not noconfig):
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

    def run(self, cmd, cwd=None, noconfig=False):
        self._decorate_tdnf_cmd_for_test(cmd, noconfig)
        return self._run(cmd, cwd=cwd)

    def _run(self, cmd, retvalonly=False, cwd=None):
        use_shell = not isinstance(cmd, list)
        print(cmd)
        process = subprocess.Popen(cmd, shell=use_shell,  # nosec
                                   stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE,
                                   cwd=cwd)
        out, err = process.communicate()
        if retvalonly:
            return {'retval': process.returncode}

        stdout = out.decode().strip()
        stderr = err.decode().strip()
        retval = process.returncode
        capture = re.search(r'^Error\((\d+)\) :', stderr, re.MULTILINE)
        if capture:
            retval = int(capture.groups()[0])

        ret = {}
        ret['stdout'] = []
        ret['stderr'] = []
        if stdout:
            ret['stdout'] = stdout.split('\n')
        if stderr:
            ret['stderr'] = stderr.split('\n')
        ret['retval'] = retval
        return ret

    # helper to create directory tree without complains when it exists:
    def makedirs(self, d):
        try:
            os.makedirs(d)
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise

    def create_repoconf(self, filename, baseurl, name):
        templ = """
[{name}]
name=Test Repo
baseurl={baseurl}
enabled=1
gpgcheck=0
metadata_expire=86400
ui_repoid_vars=basearch
"""
        with open(filename, "w") as f:
            f.write(templ.format(name=name, baseurl=baseurl))

    def _get_stdout_total_download_size(self, stdout):
        key = "Total download size"
        for line in stdout:
            if key in line:
                return line.split()[-1]

    def download_size_to_bytes(self, stdout):
        size_chars = 'bkMG'
        raw_str = self._get_stdout_total_download_size(stdout)
        size, unit = float(raw_str[:-1]), raw_str[-1]
        return size * (1024 ** size_chars.index(unit))

    def get_cached_package_sizes(self, cache_dir):
        ret = self.run(['find', cache_dir, '-name', '*.rpm'])
        pkg_size_map = {}
        for rpm in ret['stdout']:
            pkg_size_map[rpm] = os.path.getsize(rpm)
        return pkg_size_map

    def floats_approx_equal(self, x, y, tol=500):
        return abs(x - y) <= tol


@pytest.fixture(scope='session')
def utils():
    test_utils = TestUtils()
    server = Process(target=TestRepoServer,
                     args=(test_utils.config['repo_path'], ))
    server.start()

    atexit.register(StopTestRepoServer, server)

    result = -1
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    for retry in range(0, 10):
        result = sock.connect_ex(('localhost', 8080))
        if not result:
            print('Server is up and running')
            sock.close()
            break
        print('Server is not running, retrying ...')
        time.sleep(1)

    if result:
        print('Server failed to start, aborting ...')
        sys.exit(1)

    return test_utils
