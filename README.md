# tdnf - tiny dandified yum

In order to compile, from the checkout directory, run the following

```sh
mkdir build && cd build
cmake ..
make
```

Do enable debugging symbols (useful for use with `gdb`), use:
```
cmake -DCMAKE_BUILD_TYPE=Debug ..
```

You can also build tdnf using docker using the following commands:

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

## Testing

To build and run the test scripts within a container, do:

```text
export DIST=photon
docker run --security-opt seccomp:unconfined --rm -it -e DIST -v$(pwd):/build -w/build ${DIST}/tdnf-build ./ci/docker-entrypoint.sh
```
Same for
```text
export DIST=fedora
```

## Coverity

Assuming you have coverity installed on `/pathto/coverity`, you can run:

```text
docker run --rm -it -v $(pwd):/build -v /pathto/coverity/:/coverity/ -w /build photon/tdnf-build ./ci/coverity.sh
```

This will put the output to `./build-coverity`. You can then commit the results to the coverity database from that directory, or view the results in `./build-coverity/html`. For example, you can start an nginx container:

```text
docker run -it --rm -p 8080:80 --name web -v $(pwd)/build-coverity/html:/usr/share/nginx/html nginx
```
and then view results in your browser at `http://<host>:8080/`.

