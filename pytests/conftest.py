#            (c) 2019 VMware Inc.,
#
#   Author: Siddharth Chandrasekaran <csiddharth@vmware.com>

import pytest
import json
import subprocess
import os
import shutil

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

    def run(self, cmd):
        use_shell = not isinstance(cmd, list)
        process = subprocess.Popen(cmd, shell=use_shell,
                                   stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE)
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

@pytest.fixture(scope='session')
def utils():
    return TestUtils()
