# tdnf - tiny dandified yum

[![Build Status](https://travis-ci.org/vmware/tdnf.svg?branch=dev)](https://travis-ci.org/vmware/tdnf)

In order to compile, from the checkout directory, run the following

```sh
mkdir build && cd build
cmake ..
make
```

You could also build tdnf using docker using the following commands:

```sh
docker build -t photon/tdnf-build -f ci/Dockerfile.photon .
docker run --rm -it -v $(pwd):/build -w /build photon/tdnf-build
```

create a conf file named `tdnf.conf` under `/etc/tdnf/` with the following content

```text
[main]
gpgcheck=1
installonly_limit=3
clean_requirements_on_remove=true
repodir=/etc/yum.repos.d
cachedir=/var/cache/tdnf
```

Now configure repo files under `/etc/yum.repos.d` or your repodir following
`.repo` format of dnf/yum.

You should now have a client executable named tdnf under `bin/`. To test
run:

```sh
./bin/tdnf list installed
```

You should see a list of installed packages and their related info

