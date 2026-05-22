import os
import pytest
import shutil
import threading
import time
import http.server
import ssl

WORKDIR = '/root/test_ssl_config'
TEST_CONF_FILE = 'tdnf.conf'
TEST_REPO = 'test-ssl'
TEST_REPO_FILE = TEST_REPO + '.repo'
TEST_CONF_PATH = os.path.join(WORKDIR, TEST_CONF_FILE)
TEST_REPO_PATH = os.path.join(WORKDIR, TEST_REPO_FILE)

GOOD_CERT_PATH = os.path.join(WORKDIR, 'server.crt')
GOOD_KEY_PATH = os.path.join(WORKDIR, 'server.key')
BAD_CERT_PATH = os.path.join(WORKDIR, 'bad.crt')
BAD_KEY_PATH = os.path.join(WORKDIR, 'bad.key')


class QuietHandler(http.server.SimpleHTTPRequestHandler):
    def log_message(self, format, *args):
        pass


def run_server():
    server_address = ('localhost', 8443)
    httpd = http.server.HTTPServer(server_address, QuietHandler)
    context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    context.minimum_version = ssl.TLSVersion.TLSv1_2
    context.load_cert_chain(certfile=GOOD_CERT_PATH, keyfile=GOOD_KEY_PATH)
    httpd.socket = context.wrap_socket(httpd.socket, server_side=True)
    httpd.serve_forever()


@pytest.fixture(scope='module', autouse=True)
def setup_test(utils):
    os.makedirs(WORKDIR, exist_ok=True)

    # Generate certificates
    utils.run(["openssl", "genrsa", "-out", GOOD_KEY_PATH, "2048"])
    utils.run(["openssl", "req", "-new", "-x509", "-key", GOOD_KEY_PATH, "-out", GOOD_CERT_PATH, "-days", "365", "-subj", "/CN=localhost"])

    utils.run(["openssl", "genrsa", "-out", BAD_KEY_PATH, "2048"])
    utils.run(["openssl", "req", "-new", "-x509", "-key", BAD_KEY_PATH, "-out", BAD_CERT_PATH, "-days", "365", "-subj", "/CN=badhost"])

    # Start HTTPS server
    server_thread = threading.Thread(target=run_server, daemon=True)
    server_thread.start()
    time.sleep(1)

    yield
    teardown_test(utils)


def teardown_test(utils):
    if os.path.isdir(WORKDIR):
        shutil.rmtree(WORKDIR)


def run_tdnf_test(utils, global_cert=None, repo_cert=None, env_cert=None, setopt_cert=None):
    # Setup config
    with open(TEST_CONF_PATH, "w") as f:
        f.write("[main]\n")
        f.write("gpgcheck=0\n")
        f.write(f"reposdir={WORKDIR}\n")
        f.write(f"cachedir={WORKDIR}/cache\n")
        if global_cert:
            f.write(f"sslcacert={global_cert}\n")

    with open(TEST_REPO_PATH, "w") as f:
        f.write(f"[{TEST_REPO}]\n")
        f.write("name=Test Repo\n")
        f.write("baseurl=https://localhost:8443\n")
        f.write("sslverify=1\n")
        f.write("skip_if_unavailable=0\n")
        f.write("enabled=1\n")
        if repo_cert:
            f.write(f"sslcacert={repo_cert}\n")

    cmd = ['tdnf', '--config', TEST_CONF_PATH, '--refresh']
    if setopt_cert:
        cmd.append(f'--setopt=sslcacert={setopt_cert}')
    cmd.append('makecache')

    env = os.environ.copy()
    if env_cert:
        env['SSL_CERT_FILE'] = env_cert
    elif 'SSL_CERT_FILE' in env:
        del env['SSL_CERT_FILE']

    return utils.run(cmd, env=env)


def test_ssl_inherit_global(utils):
    ret = run_tdnf_test(utils, global_cert=GOOD_CERT_PATH)
    # Should get 404 because repo is empty, but SSL succeeds
    assert ret['retval'] != 0
    assert '404' in ''.join(ret['stderr'])


def test_ssl_override_global_with_good(utils):
    ret = run_tdnf_test(utils, global_cert=BAD_CERT_PATH, repo_cert=GOOD_CERT_PATH)
    assert ret['retval'] != 0
    assert '404' in ''.join(ret['stderr'])


def test_ssl_override_global_with_bad(utils):
    ret = run_tdnf_test(utils, global_cert=GOOD_CERT_PATH, repo_cert=BAD_CERT_PATH)
    assert ret['retval'] != 0
    assert 'SSL peer certificate' in ''.join(ret['stderr']) or 'curl error' in ''.join(ret['stderr'])


def test_ssl_env_only(utils):
    ret = run_tdnf_test(utils, env_cert=GOOD_CERT_PATH)
    assert ret['retval'] != 0
    assert '404' in ''.join(ret['stderr'])


def test_ssl_override_repo_with_env(utils):
    ret = run_tdnf_test(utils, repo_cert=BAD_CERT_PATH, env_cert=GOOD_CERT_PATH)
    assert ret['retval'] != 0
    assert '404' in ''.join(ret['stderr'])


def test_ssl_override_env_with_bad_setopt(utils):
    ret = run_tdnf_test(utils, env_cert=GOOD_CERT_PATH, setopt_cert=BAD_CERT_PATH)
    assert ret['retval'] != 0
    assert 'SSL peer certificate' in ''.join(ret['stderr']) or 'curl error' in ''.join(ret['stderr'])
