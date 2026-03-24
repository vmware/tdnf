#
# Copyright (C) 2026 Broadcom, Inc. All Rights Reserved.
#
# Licensed under the GNU General Public License v2 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

"""
Tests for xml:base support in repodata <location> elements.

A repository's primary.xml may carry an xml:base attribute on each
<location> element that overrides the base URL used to locate RPMs:

    <location xml:base="file:///some/pool" href="pkg.rpm"/>

libsolv stores this as SOLVABLE_MEDIABASE.  tdnf must use it when
building package URLs for list, repoquery, download, and install.

The test creates a shared RPM pool directory and three separate
repodata-only directories, each generated with a different xml:base
variant, then exercises every relevant tdnf command against both a
file:// and an http:// base URL.

Variants tested
---------------
  no_base   -- normal repo, no xml:base (regression guard)
  rel_base  -- xml:base is a relative path  (e.g. "../pool")
  abs_base  -- xml:base is an absolute path (file:// or http://)
"""

import os
import glob
import shutil
import platform
import subprocess
import tempfile
import pytest

ARCH = platform.machine()

# Package used across all subtests.  Must exist in the test repo.
PKGNAME = 'tdnf-test-one'


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _run(cmd, **kwargs):
    """Run a command and return (returncode, stdout, stderr)."""
    r = subprocess.run(cmd, capture_output=True, text=True, **kwargs)
    return r.returncode, r.stdout, r.stderr


def _createrepo(rpm_dir, baseurl=None, outputdir=None):
    """
    Run createrepo_c to index RPMs in *rpm_dir*.

    baseurl   -- passed as --baseurl; sets xml:base on every <location>
    outputdir -- write repodata here instead of inside *rpm_dir*, allowing
                 repodata and the RPM pool to live in separate directories.
    """
    cmd = ['createrepo_c', '--quiet']
    if baseurl is not None:
        cmd += ['--baseurl', baseurl]
    if outputdir is not None:
        cmd += ['--outputdir', outputdir]
    cmd.append(rpm_dir)
    rc, out, err = _run(cmd)
    assert rc == 0, f'createrepo_c failed: {err}'


def _find_rpm(repo_path, pkgname):
    """Return the path of the first RPM for *pkgname* in the built repo."""
    pattern = os.path.join(repo_path, 'build', 'RPMS', ARCH,
                           f'{pkgname}-*.rpm')
    matches = glob.glob(pattern)
    assert matches, f'No RPM found for {pkgname} under {repo_path}'
    return matches[0]


def _tdnf(utils, workdir, args):
    """Run tdnf via utils.run() with a per-test config in *workdir*."""
    conf = os.path.join(workdir, 'tdnf.conf')
    return utils.run(['tdnf', '-c', conf] + args)


def _make_tdnf_conf(workdir, cache_dir, reposdir):
    """Write a minimal tdnf.conf pointing at *reposdir*."""
    conf = os.path.join(workdir, 'tdnf.conf')
    os.makedirs(cache_dir, exist_ok=True)
    with open(conf, 'w') as f:
        f.write(f'[main]\ngpgcheck=0\nrepodir={reposdir}\ncachedir={cache_dir}\n')
    return conf


def _make_repo_file(reposdir, name, baseurl):
    """Write a .repo file for *name* pointing at *baseurl*."""
    os.makedirs(reposdir, exist_ok=True)
    with open(os.path.join(reposdir, f'{name}.repo'), 'w') as f:
        f.write(f'[{name}]\nname={name}\nbaseurl={baseurl}\n'
                f'enabled=1\ngpgcheck=0\n')


# ---------------------------------------------------------------------------
# Session-scoped fixture: build all repo layouts once
# ---------------------------------------------------------------------------

@pytest.fixture(scope='module')
def xmlbase_repos(utils):
    """
    Build a temporary workspace with:

      workdir/
        pool/            shared RPM pool (contains the actual .rpm file)
        no-base/         repodata only, no xml:base  (packages inside)
        rel-base/        repodata only, xml:base="../pool" (relative)
        abs-base/        repodata only, xml:base="file:///…/pool" (absolute)
        http-abs-base/   repodata only, xml:base="http://localhost:8080/…"

    The HTTP server started by conftest already serves from repo_path, so
    we put the pool *inside* repo_path so it is reachable over http://.
    """
    repo_path = utils.config['repo_path']
    workdir = tempfile.mkdtemp(prefix='tdnf-xmlbase-')

    try:
        # ---- pool: copy the RPM into a shared directory ----
        pool_dir = os.path.join(workdir, 'pool')
        os.makedirs(pool_dir)
        rpm_src = _find_rpm(repo_path, PKGNAME)
        rpm_name = os.path.basename(rpm_src)
        shutil.copy2(rpm_src, pool_dir)

        # The HTTP server serves repo_path as its document root.
        # Place the pool there so it is reachable via http://.
        http_pool_dir = os.path.join(repo_path, 'xmlbase-pool')
        os.makedirs(http_pool_dir, exist_ok=True)
        shutil.copy2(rpm_src, http_pool_dir)

        # ---- no-base repo: packages live next to repodata, no xml:base ----
        no_base_dir = os.path.join(workdir, 'no-base')
        os.makedirs(no_base_dir)
        shutil.copy2(rpm_src, no_base_dir)
        _createrepo(no_base_dir)

        # ---- rel-base repo: repodata in rel-base/, RPMs in pool/ ----
        # createrepo_c scans pool_dir for RPMs, writes repodata to rel_base_dir,
        # and sets xml:base="../pool" on each <location>.  The relative value is
        # resolved against the repo baseurl at runtime, so the layout is
        # relocatable.
        rel_base_dir = os.path.join(workdir, 'rel-base')
        os.makedirs(rel_base_dir)
        _createrepo(pool_dir, baseurl='../pool', outputdir=rel_base_dir)

        # ---- abs file:// base repo: xml:base = file:///…/pool ----
        abs_base_dir = os.path.join(workdir, 'abs-base')
        os.makedirs(abs_base_dir)
        _createrepo(pool_dir,
                    baseurl=f'file://{pool_dir}',
                    outputdir=abs_base_dir)

        # ---- http abs-base repo: xml:base = http://localhost:8080/xmlbase-pool ----
        http_abs_base_dir = os.path.join(workdir, 'http-abs-base')
        os.makedirs(http_abs_base_dir)
        _createrepo(http_pool_dir,
                    baseurl='http://localhost:8080/xmlbase-pool',
                    outputdir=http_abs_base_dir)

        yield {
            'workdir': workdir,
            'pool_dir': pool_dir,
            'http_pool_dir': http_pool_dir,
            'no_base_dir': no_base_dir,
            'rel_base_dir': rel_base_dir,
            'abs_base_dir': abs_base_dir,
            'http_abs_base_dir': http_abs_base_dir,
            'rpm_name': rpm_name,
        }

    finally:
        shutil.rmtree(workdir, ignore_errors=True)
        shutil.rmtree(os.path.join(repo_path, 'xmlbase-pool'),
                      ignore_errors=True)


# ---------------------------------------------------------------------------
# Per-test fixture: isolated tdnf config + cleanup
# ---------------------------------------------------------------------------

@pytest.fixture
def env(utils, xmlbase_repos):
    """
    Yield a helper that sets up an isolated tdnf environment pointing at
    one of the repo directories.  Cleans up the installed package after
    each test.
    """
    test_dirs = []

    def _make_env(repo_dir, base_url):
        """
        Create per-test tdnf config pointing at *repo_dir* via *base_url*.

        base_url is the repository base URL; the repodata is always in
        *repo_dir* itself, regardless of where the RPMs live (xml:base
        takes care of that).
        """
        envdir = tempfile.mkdtemp(dir=xmlbase_repos['workdir'],
                                  prefix='env-')
        test_dirs.append(envdir)
        reposdir = os.path.join(envdir, 'repos.d')
        cache_dir = os.path.join(envdir, 'cache')
        _make_tdnf_conf(envdir, cache_dir, reposdir)
        _make_repo_file(reposdir, 'test-repo', base_url)
        return envdir

    yield _make_env

    utils.erase_package(PKGNAME)
    for d in test_dirs:
        shutil.rmtree(d, ignore_errors=True)


# ---------------------------------------------------------------------------
# Parametrised helper: run the same assertions for every variant
# ---------------------------------------------------------------------------

def _assert_list(utils, envdir, pkgname=PKGNAME):
    """tdnf list --available must show the package."""
    conf = os.path.join(envdir, 'tdnf.conf')
    ret = utils.run(['tdnf', '-c', conf, '--disablerepo=*',
                     '--enablerepo=test-repo', 'list', '--available',
                     pkgname])
    assert ret['retval'] == 0, f'list failed: {ret["stderr"]}'
    assert pkgname in '\n'.join(ret['stdout']), \
        f'{pkgname} not found in list output'


def _repoquery_location(utils, envdir, pkgname=PKGNAME):
    """
    Run tdnf repoquery --location and return the location line.

    Filters out informational lines like "Refreshing metadata for: ..."
    so assertions are not confused by those.
    """
    conf = os.path.join(envdir, 'tdnf.conf')
    ret = utils.run(['tdnf', '-c', conf, '--disablerepo=*',
                     '--enablerepo=test-repo',
                     'repoquery', '--location', '--available', pkgname])
    assert ret['retval'] == 0, f'repoquery --location failed: {ret["stderr"]}'
    lines = [ln for ln in ret['stdout'] if ln.strip() and not ln.startswith('Refreshing')]
    assert lines, f'repoquery --location produced no location output; stdout={ret["stdout"]}'
    return lines[-1].strip()


def _assert_repoquery_location(utils, envdir, expected_suffix,
                               pkgname=PKGNAME):
    """
    tdnf repoquery --location must return a path/URL ending with
    *expected_suffix* (the RPM filename).
    """
    location = _repoquery_location(utils, envdir, pkgname)
    assert location.endswith(expected_suffix), (
        f'repoquery --location {location!r} does not end with '
        f'{expected_suffix!r}'
    )


def _assert_install(utils, envdir, pkgname=PKGNAME):
    """tdnf install -y must succeed and the package must be installed."""
    conf = os.path.join(envdir, 'tdnf.conf')
    utils.erase_package(pkgname)
    ret = utils.run(['tdnf', '-c', conf, '--disablerepo=*',
                     '--enablerepo=test-repo',
                     'install', '-y', '--nogpgcheck', pkgname])
    assert ret['retval'] == 0, f'install failed: {ret["stderr"]}'
    assert utils.check_package(pkgname), f'{pkgname} not installed after install'
    utils.erase_package(pkgname)


def _assert_downloadonly(utils, envdir, pkgname=PKGNAME):
    """tdnf install --downloadonly must download the RPM without installing."""
    conf = os.path.join(envdir, 'tdnf.conf')
    utils.erase_package(pkgname)
    dldir = tempfile.mkdtemp(dir=envdir, prefix='dl-')
    ret = utils.run(['tdnf', '-c', conf, '--disablerepo=*',
                     '--enablerepo=test-repo',
                     'install', '-y', '--nogpgcheck',
                     '--downloadonly', '--downloaddir', dldir,
                     pkgname])
    assert ret['retval'] == 0, f'--downloadonly failed: {ret["stderr"]}'
    rpms = glob.glob(os.path.join(dldir, '*.rpm'))
    assert rpms, f'No RPM downloaded to {dldir}'
    assert not utils.check_package(pkgname), \
        f'{pkgname} was installed despite --downloadonly'


# ---------------------------------------------------------------------------
# Tests: no xml:base (regression guard)
# ---------------------------------------------------------------------------

class TestNoBase:
    """Normal repo with no xml:base — must keep working as before."""

    def test_list(self, utils, env, xmlbase_repos):
        d = env(xmlbase_repos['no_base_dir'],
                f'file://{xmlbase_repos["no_base_dir"]}')
        _assert_list(utils, d)

    def test_repoquery_location(self, utils, env, xmlbase_repos):
        d = env(xmlbase_repos['no_base_dir'],
                f'file://{xmlbase_repos["no_base_dir"]}')
        _assert_repoquery_location(utils, d, xmlbase_repos['rpm_name'])

    def test_install_file(self, utils, env, xmlbase_repos):
        d = env(xmlbase_repos['no_base_dir'],
                f'file://{xmlbase_repos["no_base_dir"]}')
        _assert_install(utils, d)

    def test_install_http(self, utils, env, xmlbase_repos):
        # Serve no-base dir via the test HTTP server.
        # The conftest server serves from repo_path; copy repodata there.
        repo_path = utils.config['repo_path']
        http_dir = os.path.join(repo_path, 'xmlbase-no-base')
        shutil.copytree(xmlbase_repos['no_base_dir'], http_dir,
                        dirs_exist_ok=True)
        try:
            d = env(http_dir, 'http://localhost:8080/xmlbase-no-base')
            _assert_install(utils, d)
        finally:
            shutil.rmtree(http_dir, ignore_errors=True)

    def test_downloadonly(self, utils, env, xmlbase_repos):
        d = env(xmlbase_repos['no_base_dir'],
                f'file://{xmlbase_repos["no_base_dir"]}')
        _assert_downloadonly(utils, d)


# ---------------------------------------------------------------------------
# Tests: relative xml:base
# ---------------------------------------------------------------------------

class TestRelativeBase:
    """
    Repodata carries xml:base="../pool" (relative).
    The base URL points at the rel-base directory; tdnf must resolve
    pool/ relative to that base URL.
    """

    def test_list(self, utils, env, xmlbase_repos):
        d = env(xmlbase_repos['rel_base_dir'],
                f'file://{xmlbase_repos["rel_base_dir"]}')
        _assert_list(utils, d)

    def test_repoquery_location(self, utils, env, xmlbase_repos):
        d = env(xmlbase_repos['rel_base_dir'],
                f'file://{xmlbase_repos["rel_base_dir"]}')
        _assert_repoquery_location(utils, d, xmlbase_repos['rpm_name'])

    def test_install_file(self, utils, env, xmlbase_repos):
        d = env(xmlbase_repos['rel_base_dir'],
                f'file://{xmlbase_repos["rel_base_dir"]}')
        _assert_install(utils, d)

    def test_install_http(self, utils, env, xmlbase_repos):
        repo_path = utils.config['repo_path']
        # Place the RPM pool as a subdirectory of the repo base so the
        # relative xml:base "pool/" resolves without parent-dir traversal.
        # Layout (all served by the test HTTP server):
        #   xmlbase-rel-http/           <- repo baseurl
        #   xmlbase-rel-http/pool/      <- xml:base "pool/" resolves here
        #   xmlbase-rel-http/repodata/  <- generated metadata
        http_base = os.path.join(repo_path, 'xmlbase-rel-http')
        http_pool = os.path.join(http_base, 'pool')
        os.makedirs(http_pool, exist_ok=True)
        shutil.copy2(
            os.path.join(xmlbase_repos['pool_dir'], xmlbase_repos['rpm_name']),
            http_pool
        )
        _createrepo(http_pool,
                    baseurl='pool',
                    outputdir=http_base)
        try:
            d = env(http_base, 'http://localhost:8080/xmlbase-rel-http')
            _assert_install(utils, d)
        finally:
            shutil.rmtree(http_base, ignore_errors=True)

    def test_downloadonly(self, utils, env, xmlbase_repos):
        d = env(xmlbase_repos['rel_base_dir'],
                f'file://{xmlbase_repos["rel_base_dir"]}')
        _assert_downloadonly(utils, d)


# ---------------------------------------------------------------------------
# Tests: absolute xml:base — file://
# ---------------------------------------------------------------------------

class TestAbsoluteFileBase:
    """
    Repodata carries xml:base="file:///…/pool" (absolute file:// URL).
    The base URL used by tdnf can point anywhere; the xml:base fully
    determines the package location.
    """

    def test_list(self, utils, env, xmlbase_repos):
        d = env(xmlbase_repos['abs_base_dir'],
                f'file://{xmlbase_repos["abs_base_dir"]}')
        _assert_list(utils, d)

    def test_repoquery_location(self, utils, env, xmlbase_repos):
        d = env(xmlbase_repos['abs_base_dir'],
                f'file://{xmlbase_repos["abs_base_dir"]}')
        # Location must be an absolute file:// URL pointing into pool_dir
        loc = _repoquery_location(utils, d)
        assert loc.startswith('file://'), \
            f'expected file:// URL, got: {loc!r}'
        assert loc.endswith(xmlbase_repos['rpm_name']), \
            f'expected URL ending in {xmlbase_repos["rpm_name"]!r}, got: {loc!r}'
        assert xmlbase_repos['pool_dir'] in loc, \
            f'expected pool_dir in URL, got: {loc!r}'

    def test_install_file(self, utils, env, xmlbase_repos):
        d = env(xmlbase_repos['abs_base_dir'],
                f'file://{xmlbase_repos["abs_base_dir"]}')
        _assert_install(utils, d)

    def test_install_repodata_via_http(self, utils, env, xmlbase_repos):
        """Repodata served via HTTP, but xml:base points at local file://."""
        repo_path = utils.config['repo_path']
        http_dir = os.path.join(repo_path, 'xmlbase-abs-base')
        shutil.copytree(xmlbase_repos['abs_base_dir'], http_dir,
                        dirs_exist_ok=True)
        try:
            # baseurl is http (for repodata fetch), but packages come from
            # the absolute file:// xml:base
            d = env(http_dir, 'http://localhost:8080/xmlbase-abs-base')
            _assert_install(utils, d)
        finally:
            shutil.rmtree(http_dir, ignore_errors=True)

    def test_downloadonly(self, utils, env, xmlbase_repos):
        d = env(xmlbase_repos['abs_base_dir'],
                f'file://{xmlbase_repos["abs_base_dir"]}')
        _assert_downloadonly(utils, d)


# ---------------------------------------------------------------------------
# Tests: absolute xml:base — http://
# ---------------------------------------------------------------------------

class TestAbsoluteHttpBase:
    """
    Repodata carries xml:base="http://localhost:8080/xmlbase-pool".
    Packages are fetched from the HTTP server regardless of the repo baseurl.
    """

    def test_list(self, utils, env, xmlbase_repos):
        d = env(xmlbase_repos['http_abs_base_dir'],
                f'file://{xmlbase_repos["http_abs_base_dir"]}')
        _assert_list(utils, d)

    def test_repoquery_location(self, utils, env, xmlbase_repos):
        d = env(xmlbase_repos['http_abs_base_dir'],
                f'file://{xmlbase_repos["http_abs_base_dir"]}')
        loc = _repoquery_location(utils, d)
        assert loc.startswith('http://localhost:8080/xmlbase-pool'), \
            f'expected http://localhost:8080/xmlbase-pool URL, got: {loc!r}'
        assert loc.endswith(xmlbase_repos['rpm_name']), \
            f'expected URL ending in RPM name, got: {loc!r}'

    def test_install(self, utils, env, xmlbase_repos):
        """Install using absolute http:// xml:base, repo itself via file://."""
        d = env(xmlbase_repos['http_abs_base_dir'],
                f'file://{xmlbase_repos["http_abs_base_dir"]}')
        _assert_install(utils, d)

    def test_install_both_http(self, utils, env, xmlbase_repos):
        """Both repodata and packages served via HTTP."""
        repo_path = utils.config['repo_path']
        http_dir = os.path.join(repo_path, 'xmlbase-http-abs-base')
        shutil.copytree(xmlbase_repos['http_abs_base_dir'], http_dir,
                        dirs_exist_ok=True)
        try:
            d = env(http_dir, 'http://localhost:8080/xmlbase-http-abs-base')
            _assert_install(utils, d)
        finally:
            shutil.rmtree(http_dir, ignore_errors=True)

    def test_downloadonly(self, utils, env, xmlbase_repos):
        d = env(xmlbase_repos['http_abs_base_dir'],
                f'file://{xmlbase_repos["http_abs_base_dir"]}')
        _assert_downloadonly(utils, d)
