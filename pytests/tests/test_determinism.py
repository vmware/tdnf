import os
import json
import pytest


# Regression tests for the directory-listing determinism fixes in
# client/repolist.c, client/plugins.c and client/config.c: these switched
# from raw opendir()/readdir() (whose order is filesystem-dependent and not
# guaranteed stable) to scandir()+alphasort(), so repo config discovery,
# plugin config discovery, and config drop-in ("*.d") discovery all process
# entries in a fixed, alphabetically-sorted order.
#
# Each test below deliberately creates the "high" (zzz-ish) filename before
# the "low" (aaa-ish) one, i.e. reverse-alphabetical creation order, so that
# a regression back to unsorted readdir() would tend to surface the entries
# in creation order (high before low) on filesystems that return readdir()
# entries roughly in creation order for small directories - and thus be
# caught by asserting the low entry is processed/observed first.

REPO_LOW = 'aaa-determinism'
REPO_HIGH = 'zzz-determinism'

PLUGIN_LOW = 'aaa_plugin_determinism'
PLUGIN_HIGH = 'zzz_plugin_determinism'


@pytest.fixture(scope='function', autouse=True)
def setup_test(utils):
    yield
    teardown_test(utils)


def teardown_test(utils):
    repo_dir = os.path.join(utils.config['repo_path'], 'yum.repos.d')
    for repoid in (REPO_LOW, REPO_HIGH):
        fn = os.path.join(repo_dir, '{}.repo'.format(repoid))
        if os.path.isfile(fn):
            os.remove(fn)

    plugin_conf_path = os.path.join(utils.config['repo_path'], 'pluginconf.d')
    for name in (PLUGIN_LOW, PLUGIN_HIGH):
        fn = os.path.join(plugin_conf_path, name + '.conf')
        if os.path.isfile(fn):
            os.remove(fn)
    utils.edit_config({'plugins': None, 'pluginconfpath': None})

    locks_dir = os.path.join(utils.config['repo_path'], 'locks.d')
    for name in ('zzz-determinism.conf', 'aaa-determinism.conf'):
        fn = os.path.join(locks_dir, name)
        if os.path.isfile(fn):
            os.remove(fn)

    utils.erase_package(utils.config['sglversion_pkgname'])
    utils.erase_package(utils.config['sglversion2_pkgname'])


def find_repo_index(repolist, repoid):
    for i, repo in enumerate(repolist):
        if repo['Repo'] == repoid:
            return i
    return None


# client/repolist.c: TDNFLoadRepoData() must process *.repo files from
# reposdir in alphabetical filename order, independent of on-disk order.
def test_repolist_dir_order_is_alphabetical(utils):
    repo_dir = os.path.join(utils.config['repo_path'], 'yum.repos.d')

    for repoid in (REPO_HIGH, REPO_LOW):
        utils.edit_config(
            {
                'name': repoid,
                'enabled': '1',
                'baseurl': 'http://pkgs.{}.org/repo'.format(repoid),
            },
            section=repoid,
            filename=os.path.join(repo_dir, '{}.repo'.format(repoid)),
        )

    ret = utils.run(['tdnf', 'repolist', 'all', '-j'])
    assert ret['retval'] == 0
    repolist = json.loads('\n'.join(ret['stdout']))

    idx_low = find_repo_index(repolist, REPO_LOW)
    idx_high = find_repo_index(repolist, REPO_HIGH)
    assert idx_low is not None
    assert idx_high is not None
    # tdnf repolist prints repos in the order they were loaded, with no
    # further re-sorting (tools/cli/lib/api.c), so this directly reflects
    # TDNFLoadRepoData()'s directory-scan order.
    assert idx_low < idx_high


# client/plugins.c: _TDNFLoadPluginConfigs() must process plugin *.conf
# files in alphabetical filename order, independent of on-disk order.
# Neither fake plugin has a matching .so, so tdnf attempts and fails to
# load each one in turn, printing one "Error loading plugin: .../lib<name>.so"
# line per plugin - in the order plugin configs were processed.
def test_plugin_conf_dir_order_is_alphabetical(utils):
    plugin_conf_path = os.path.join(utils.config['repo_path'], 'pluginconf.d')
    os.makedirs(plugin_conf_path, exist_ok=True)

    utils.edit_config({'plugins': '1', 'pluginconfpath': plugin_conf_path})

    for name in (PLUGIN_HIGH, PLUGIN_LOW):
        with open(os.path.join(plugin_conf_path, name + '.conf'), 'w') as f:
            f.write('[main]\nenabled=1\n')

    ret = utils.run(['tdnf', 'repolist'])
    assert ret['retval'] == 0

    load_errors = [line for line in ret['stderr'] if line.startswith('Error loading plugin')]
    assert len(load_errors) == 2
    assert PLUGIN_LOW in load_errors[0]
    assert PLUGIN_HIGH in load_errors[1]


# client/config.c: TDNFReadConfFilesFromDir() (used for locks.d,
# minversions.d and protected.d) was rewritten from a fragile two-pass
# opendir()/readdir() (count, then re-open and re-read - a TOCTOU risk if
# the directory changed in between) to a single sorted scandir() pass.
# This is a regression test for that rewrite: with multiple drop-in files
# present, all of them must still be read and applied, regardless of
# on-disk order or how many *.conf files exist in the directory.
def test_locks_multiple_dropin_files_all_applied(utils):
    dirname = os.path.join(utils.config['repo_path'], 'locks.d')
    os.makedirs(dirname, exist_ok=True)

    pkg1 = utils.config['sglversion_pkgname']
    pkg2 = utils.config['sglversion2_pkgname']

    with open(os.path.join(dirname, 'zzz-determinism.conf'), 'w') as f:
        f.write(pkg1 + '\n')
    with open(os.path.join(dirname, 'aaa-determinism.conf'), 'w') as f:
        f.write(pkg2 + '\n')

    utils.install_package(pkg1)
    utils.install_package(pkg2)
    assert utils.check_package(pkg1)
    assert utils.check_package(pkg2)

    utils.run(['tdnf', '-y', '--nogpgcheck', 'remove', pkg1])
    utils.run(['tdnf', '-y', '--nogpgcheck', 'remove', pkg2])

    # both locks must be honored, regardless of drop-in file processing order
    assert utils.check_package(pkg1)
    assert utils.check_package(pkg2)
