#
# Copyright (C) 2026 Broadcom, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

"""
Downloads are written to an exclusively created temporary file next to the
destination and renamed into place. A pre-existing "<file>.tmp" (for example
a symlink planted in a writable download directory) must never be followed,
and temporary files must not be left behind after failures or retries.
"""

from pathlib import Path
from functools import partial
from http.server import SimpleHTTPRequestHandler, HTTPServer
import platform
import shutil
import stat
import tempfile
import threading

import pytest

PKGNAME = 'tdnf-test-one'
REPONAME = 'download-tempfile'


@pytest.fixture
def package_repo(utils, tmp_path):
    """Serve one RPM from its own repository with an isolated tdnf config."""
    repo_path = Path(utils.config['repo_path'])
    arch = platform.machine()
    rpms = sorted((repo_path / 'build' / 'RPMS' / arch).glob(f'{PKGNAME}-*.rpm'))
    assert rpms, f'No {PKGNAME} RPM found in the test repository'

    # inside repo_path, so that the test HTTP server serves it
    with tempfile.TemporaryDirectory(prefix='download-tempfile-', dir=repo_path) as workdir:
        workdir = Path(workdir)
        repo_dir = workdir / 'repo'
        (repo_dir / 'RPMS').mkdir(parents=True)
        source = repo_dir / 'RPMS' / rpms[0].name
        shutil.copy2(rpms[0], source)
        ret = utils.run(['createrepo_c', '--quiet', str(repo_dir)])
        assert ret['retval'] == 0, ret

        reposdir = tmp_path / 'repos.d'
        reposdir.mkdir()
        conf = tmp_path / 'tdnf.conf'
        utils.edit_config({'gpgcheck': '0', 'repodir': str(reposdir),
                           'cachedir': str(tmp_path / 'cache'), 'plugins': '0'},
                          section='main', filename=str(conf))
        baseurl = f'http://localhost:8080/{workdir.name}/repo'
        utils.create_repoconf(str(reposdir / f'{REPONAME}.repo'), baseurl, REPONAME)
        yield {
            'command': ['tdnf', '-c', str(conf), f'--repo={REPONAME}'],
            'source': source,
            'location': Path('RPMS') / rpms[0].name,
            'directory': repo_dir,
        }


@pytest.fixture
def interrupted_package_repo(package_repo, request):
    """Interrupt the first RPM response, or every response when parametrized True."""
    state = {'attempts': 0}
    always_interrupt = getattr(request, 'param', False)

    class Handler(SimpleHTTPRequestHandler):
        def do_GET(self):
            if self.path.endswith('.rpm'):
                state['attempts'] += 1
                if state['attempts'] == 1 or always_interrupt:
                    # Send more bytes than the complete RPM, so that a retry
                    # reusing the partial file would leave trailing data.
                    body = package_repo['source'].read_bytes() + b'x' * 8192
                    self.send_response(200)
                    self.send_header('Content-Length', str(len(body) + 1))
                    self.end_headers()
                    self.wfile.write(body)
                    self.close_connection = True
                    return
            super().do_GET()

    handler = partial(Handler, directory=str(package_repo['directory']))
    with HTTPServer(('127.0.0.1', 0), handler) as server:
        thread = threading.Thread(target=server.serve_forever, daemon=True)
        thread.start()
        try:
            yield {
                'command': package_repo['command'] + [
                    f'--setopt={REPONAME}.baseurl=http://127.0.0.1:{server.server_port}',
                    f'--setopt={REPONAME}.retries=1',
                    f'--setopt={REPONAME}.timeout=5'],
                'state': state,
                'always_interrupt': always_interrupt,
            }
        finally:
            server.shutdown()
            thread.join()


@pytest.mark.parametrize('operation', ['reposync', 'downloadonly'])
def test_download_ignores_predictable_temp_symlink(utils, tmp_path, package_repo, operation):
    destination = tmp_path / 'downloads'
    destination.mkdir(mode=0o777)
    if operation == 'reposync':
        downloaded = destination / package_repo['location']
        args = ['reposync', '--norepopath', f'--download-path={destination}']
    else:
        downloaded = destination / package_repo['source'].name
        args = ['install', '-y', '--downloadonly', '--alldeps',
                f'--downloaddir={destination}', PKGNAME]
    downloaded.parent.mkdir(parents=True, exist_ok=True)

    victim = tmp_path / 'victim'
    victim.write_bytes(b'original contents\n')
    victim.chmod(0o600)
    legacy_temp = downloaded.with_name(downloaded.name + '.tmp')
    legacy_temp.symlink_to(victim)

    ret = utils.run(package_repo['command'] + args)

    assert ret['retval'] == 0, ret
    assert victim.read_bytes() == b'original contents\n'
    assert stat.S_IMODE(victim.stat().st_mode) == 0o600
    assert legacy_temp.is_symlink()
    assert not downloaded.is_symlink()
    assert downloaded.read_bytes() == package_repo['source'].read_bytes()
    assert stat.S_IMODE(downloaded.stat().st_mode) == 0o644
    assert set(downloaded.parent.iterdir()) == {downloaded, legacy_temp}


def test_failed_download_preserves_temp_symlink(utils, tmp_path, package_repo):
    destination = tmp_path / 'repo'
    downloaded = destination / package_repo['location']
    downloaded.parent.mkdir(parents=True)
    victim = tmp_path / 'victim'
    victim.write_bytes(b'original contents\n')
    legacy_temp = downloaded.with_name(downloaded.name + '.tmp')
    legacy_temp.symlink_to(victim)
    package_repo['source'].unlink()  # Metadata still advertises the RPM.

    ret = utils.run(package_repo['command'] + [
        'reposync', '--norepopath', f'--download-path={destination}'])

    assert ret['retval'] != 0, ret
    assert victim.read_bytes() == b'original contents\n'
    assert legacy_temp.is_symlink()
    assert not downloaded.exists()
    assert set(downloaded.parent.iterdir()) == {legacy_temp}


@pytest.mark.parametrize('interrupted_package_repo', [False, True], indirect=True)
def test_download_retry_discards_partial_and_cleans_temp_file(
        utils, tmp_path, package_repo, interrupted_package_repo):
    destination = tmp_path / 'repo'
    ret = utils.run(interrupted_package_repo['command'] + [
        'reposync', '--norepopath', f'--download-path={destination}'])

    assert interrupted_package_repo['state']['attempts'] == 2
    downloaded = destination / package_repo['location']
    if interrupted_package_repo['always_interrupt']:
        assert ret['retval'] != 0, ret
        assert not downloaded.exists()
        assert not list(downloaded.parent.iterdir())
    else:
        assert ret['retval'] == 0, ret
        assert downloaded.read_bytes() == package_repo['source'].read_bytes()
        assert set(downloaded.parent.iterdir()) == {downloaded}
