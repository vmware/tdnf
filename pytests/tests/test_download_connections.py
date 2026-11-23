#
# Copyright (C) 2026 Broadcom, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

"""
Downloads share connections to the server instead of opening a new
connection (and doing a new TLS handshake) for each file. Packages are
downloaded in parallel, at most max_parallel_downloads at a time.
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
# long enough for transfers to overlap
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


def download_only(env, parallel, *args):
    return env['command'] + [f'--setopt=max_parallel_downloads={parallel}',
                             'install', '-y', '--downloadonly', *args,
                             *PKGNAMES]


def assert_downloaded(env, directory):
    for source in env['sources'].values():
        downloaded = directory / source.name
        assert downloaded.read_bytes() == source.read_bytes()
    assert not [p for p in directory.iterdir() if '.tmp' in p.name]


@pytest.mark.parametrize('parallel', [1, 3])
def test_parallel_download_limit(utils, env, parallel):
    ret = utils.run(download_only(env, parallel))
    assert ret['retval'] == 0, ret

    server = env['server']
    assert server.rpm_requests == len(PKGNAMES)
    assert server.max_inflight == parallel
    # metadata and packages share connections instead of one per file
    if parallel == 1:
        assert server.connections == 1
    else:
        assert server.connections <= parallel
    assert_downloaded(env, env['rpm_cache'])


def test_parallel_download_install(utils, env):
    ret = utils.run(env['command'] + ['install', '-y', *PKGNAMES])
    assert ret['retval'] == 0, ret
    assert env['server'].max_inflight > 1
    for name in PKGNAMES:
        assert utils.check_package(name)


def test_parallel_download_to_directory(utils, env, tmp_path):
    downloaddir = tmp_path / 'downloads'
    downloaddir.mkdir()

    ret = utils.run(download_only(env, 3, f'--downloaddir={downloaddir}'))
    assert ret['retval'] == 0, ret
    assert env['server'].max_inflight == 3
    assert_downloaded(env, downloaddir)

    env['server'].reset()
    ret = utils.run(download_only(env, 3, f'--downloaddir={downloaddir}'))
    assert ret['retval'] == 0, ret
    assert env['server'].rpm_requests == 0
    for name in PKGNAMES:
        assert f'{name} package already downloaded' in ret['stdout']


def test_parallel_download_failure(utils, env):
    server = env['server']
    failing = env['sources'][PKGNAMES[0]].name
    server.fail = failing

    ret = utils.run(download_only(env, 3))
    assert ret['retval'] != 0
    assert (f'Error: 404 when downloading {server.url}/RPMS/{failing}'
            in '\n'.join(ret['stderr']))
    # transfers aborted by the failure leave no temporary files behind
    assert not [p for p in env['rpm_cache'].iterdir() if '.tmp' in p.name]
    assert not (env['rpm_cache'] / failing).exists()
