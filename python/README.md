# tdnf python interface
tdnf python interfaces will work for python2 and python3. details as follows.

### api
- [repolist](#repolist)
- [install](#install)
- [update](#update)
- [downgrade](#downgrade)
- [erase](#erase)
- [distro-sync](#distro-sync)

## repolist
returns a python list of repodata types.

### parameter (optional): filter
```
    REPOLISTFILTER_ALL = 0
    REPOLISTFILTER_DISABLED = 2
    REPOLISTFILTER_ENABLED = 1
```

example:
```
root [ / ]# python3
Python 3.7.3 (default, Jun 20 2019, 03:44:05) 
[GCC 7.3.0] on linux
Type "help", "copyright", "credits" or "license" for more information.
>>> import tdnf
>>> tdnf.repolist()
[{id: photon-updates, name: VMware Photon Linux 3.0(x86_64) Updates, baseurl: https://dl.bintray.com/vmware/photon_updates_3.0_x86_64, enabled: 1}, {id: photon, name: VMware Photon Linux 3.0(x86_64), baseurl: https://dl.bintray.com/vmware/photon_release_3.0_x86_64, enabled: 1}, {id: photon-extras, name: VMware Photon Extras 3.0(x86_64), baseurl: https://dl.bintray.com/vmware/photon_extras_3.0_x86_64, enabled: 1}]
>>> tdnf.repolist(filter=tdnf.REPOLISTFILTER_DISABLED)
[{id: photon-iso, name: VMWare Photon Linux ISO 3.0(x86_64), baseurl: file:///mnt/cdrom/RPMS, enabled: 0}, {id: photon-debuginfo, name: VMware Photon Linux debuginfo 3.0(x86_64), baseurl: https://dl.bintray.com/vmware/photon_debuginfo_$releasever_$basearch, enabled: 0}]
```

##install
installs a list of packages and their depedencies. updates installed packages if there are updates available.

##parameter (required): pkgs
```
tdnf.install(pkgs=['curl','wget'])
```

##parameter (optional): quiet
```
tdnf.install(pkgs=['curl','wget'], quiet=True)
```

##parameter (optional): refresh
```
tdnf.install(pkgs=['curl','wget'], refresh=True)
```

##update
updates specified packages or all packages that have updates.

##parameter (optional): pkgs
```
tdnf.update(pkgs=['curl','wget'])
```

##parameter (optional): quiet
```
tdnf.update(pkgs=['curl','wget'], quiet=True)
```

##parameter (optional): refresh
```
tdnf.update(pkgs=['curl','wget'], refresh=True)
```

##downgrade
downgrades specified packages or all packages that have a downgrade path.

##parameter (optional): pkgs
```
tdnf.downgrade(pkgs=['curl','wget'])
```

##parameter (optional): quiet
```
tdnf.downgrade(pkgs=['curl','wget'], quiet=True)
```

##parameter (optional): refresh
```
tdnf.downgrade(pkgs=['curl','wget'], refresh=True)
```

##erase
remove specified packages and their dependencies

##parameter (required): pkgs
```
tdnf.erase(pkgs=['wget'])
```

##parameter (optional): quiet
```
tdnf.erase(pkgs=['wget'], quiet=True)
```

##parameter (optional): refresh
```
tdnf.erase(pkgs=['wget'], refresh=True)
```

##erase
synchronize installed packages to the latest available versions

##parameter (optional): quiet
```
tdnf.distro-sync(quiet=True)
```

##parameter (optional): refresh
```
tdnf.distro-sync(refresh=True)
```
