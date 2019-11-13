
def test_assumeno_install(utils):
    pkgname = utils.config["sglversion_pkgname"]
    utils.run([ 'tdnf', '--assumeno', 'install', pkgname ])

    # check
    ret = utils.run([ 'tdnf', 'list', pkgname ])
    Found = False
    for line in ret['stdout']:
        if pkgname in line and '@System' in line:
            found = True
            break
    assert (found == True)

def test_assumeno_erase(utils):
    pkgname = utils.config["sglversion_pkgname"]
    utils.run([ 'tdnf', '--assumeno',  'erase', pkgname ])

    # check
    ret = utils.run([ 'tdnf', 'list', pkgname ])
    Found = False
    for line in ret['stdout']:
        if pkgname in line and '@System' in line:
            found = True
            break
    assert (found == True)

