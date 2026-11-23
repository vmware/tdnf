#
# Copyright (C) 2026 Broadcom, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

"""
Downloads share connections to the server instead of opening a new
connection (and doing a new TLS handshake) for each file.
"""

from functools import partial
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
import shutil
import threading
import time

import pytest

REPONAME = 'download-connections'
PKGNAMES = ['tdnf-test-one', 'tdnf-test-two', 'tdnf-test3', 'tdnf-test4',
            'tdnf-test-noarch', 'tdnf-test-doc', 'tdnf-test-epoch']
# long enough for transfers to overlap, if they do
RPM_DELAY = 0.3


class Server:
    """HTTP/1.1 server that records how many RPM requests overlap."""

    def __init__(self, directory):
        self.lock = threading.Lock()
        self.inflight = 0
        self.max_inflight = 0
        self.connections = 0
        self.rpm_requests = 0
        self.fail = None    # RPM file name to answer with 404
        server = self

        class Handler(SimpleHTTPRequestHandler):
            protocol_version = 'HTTP/1.1'   # keep connections open

            def log_message(self, *args):
                pass

            def setup(self):
                super().setup()
                with server.lock:
                    server.connections += 1

            def do_GET(self):
                if not self.path.endswith('.rpm'):
                    super().do_GET()
                    return
                with server.lock:
                    server.rpm_requests += 1
                    server.inflight += 1
                    server.max_inflight = max(server.max_inflight,
                                              server.inflight)
                try:
                    time.sleep(RPM_DELAY)
                    if server.fail and self.path.endswith('/' + server.fail):
                        self.send_error(404)
                    else:
                        super().do_GET()
                finally:
                    with server.lock:
                        server.inflight -= 1

        self.httpd = ThreadingHTTPServer(
            ('127.0.0.1', 0), partial(Handler, directory=str(directory)))
        self.httpd.daemon_threads = True
        self.thread = threading.Thread(target=self.httpd.serve_forever,
                                       daemon=True)

    @property
    def url(self):
        return f'http://127.0.0.1:{self.httpd.server_port}'

    def reset(self):
        with self.lock:
            self.max_inflight = 0
            self.connections = 0
            self.rpm_requests = 0


@pytest.fixture
def env(utils, tmp_path):
    repo_dir = tmp_path / 'repo'
    (repo_dir / 'RPMS').mkdir(parents=True)
    sources = {}
    for name in PKGNAMES:
        rpm = Path(utils.find_rpm(name))
        sources[name] = repo_dir / 'RPMS' / rpm.name
        shutil.copy2(rpm, sources[name])
    ret = utils.run(['createrepo_c', '--quiet', str(repo_dir)])
    assert ret['retval'] == 0, ret

    server = Server(repo_dir)
    server.thread.start()

    reposdir = tmp_path / 'repos.d'
    reposdir.mkdir()
    cachedir = tmp_path / 'cache'
    conf = tmp_path / 'tdnf.conf'
    utils.edit_config({'gpgcheck': '0', 'repodir': str(reposdir),
                       'cachedir': str(cachedir), 'plugins': '0'},
                      section='main', filename=str(conf))
    utils.create_repoconf(str(reposdir / f'{REPONAME}.repo'), server.url,
                          REPONAME)
    try:
        yield {
            'command': ['tdnf', '-c', str(conf), f'--repo={REPONAME}'],
            'server': server,
            'sources': sources,
            'rpm_cache': cachedir / REPONAME / 'rpms' / 'RPMS',
        }
    finally:
        server.httpd.shutdown()
        server.thread.join()
        server.httpd.server_close()
        for name in PKGNAMES:
            utils.erase_package(name)


def download_only(env, *args):
    return env['command'] + ['install', '-y', '--downloadonly', *args,
                             *PKGNAMES]


def assert_downloaded(env, directory):
    for source in env['sources'].values():
        downloaded = directory / source.name
        assert downloaded.read_bytes() == source.read_bytes()
    assert not [p for p in directory.iterdir() if '.tmp' in p.name]


def test_downloads_share_connection(utils, env):
    ret = utils.run(download_only(env))
    assert ret['retval'] == 0, ret

    server = env['server']
    assert server.rpm_requests == len(PKGNAMES)
    assert server.max_inflight == 1
    # metadata and all packages over one connection
    assert server.connections == 1
    assert_downloaded(env, env['rpm_cache'])
