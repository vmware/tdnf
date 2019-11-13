import pytest
import json
import subprocess
import os
import shutil
import logging

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
        self.logger = self.get_logger()
        self.config = JsonWrapper('config.json').read()

    def string_to_loglevel(self, loglevel):
        logLevelMap = {
            "error": logging.ERROR,
            "warning": logging.WARNING,
            "info": logging.INFO,
            "debug": logging.DEBUG,
        }
        return logLevelMap.get(loglevel, logging.INFO)

    def get_logger(self, logpath=None, loglevel="debug", console=False):
        logger = logging.getLogger("installer")
        if not logger.handlers:
            # file handler
            logfile = "installer.log"
            if logpath is not None:
                if not os.path.isdir(logpath):
                    os.makedirs(logpath)
                logfile = logpath + "/" + logfile
            fhandler = logging.FileHandler(logfile)
            fhformatter = logging.Formatter('%(asctime)s - %(message)s')
            fhandler.setFormatter(fhformatter)
            #fhandler.setLevel(logging.DEBUG)
            logger.addHandler(fhandler)

            # console handler
            if console:
                ch = logging.StreamHandler()
                if loglevel=="debug":
                    chformatter = logging.Formatter('%(asctime)s - %(message)s')
                else:
                    chformatter = logging.Formatter('%(message)s')
                ch.setFormatter(chformatter)
                #ch.setLevel(Logger.string_to_loglevel(loglevel))
                logger.addHandler(ch)

            logger.setLevel(self.string_to_loglevel(loglevel))
            logger.debug("-" * 75)
            logger.debug("Starting Log")
            logger.debug("-" * 75)
        return logger

    def run(self, cmd):
        use_shell = not isinstance(cmd, list)
        process = subprocess.Popen(cmd, shell=use_shell,
                                   stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE)
        out, err = process.communicate()
        ret = {}
        ret['retval'] = process.returncode
        ret['stdout'] = out.decode().split('\n')
        ret['strerr'] = err.decode().split('\n')

        return ret

@pytest.fixture
def utils():
    return TestUtils()
