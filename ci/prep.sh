#!/bin/bash

set -ex

rel_file="/etc/os-release"

common_pkgs=(
  cmake
  createrepo_c
  e2fsprogs
  expat-devel
  findutils
  gpgme-devel
  libsolv-devel
  openssl-devel
  popt-devel
  python3-devel
  python3-pip
  python3-pyOpenSSL
  python3-pytest
  python3-requests
  python3-setuptools
  python3-urllib3
  rpm-build
  sed
  sqlite-devel
  sudo
  util-linux
  valgrind
  which
)

if grep -qw "Fedora" ${rel_file}; then
  fedora_packages=(
    ${common_pkgs[@]}
    gcc
    glib2-devel
    libcurl-devel
    make
    python3-flake8
    rpm-devel
    rpm-sign
    shadow-utils
  )
  dnf -y upgrade --refresh
  dnf -y install ${fedora_packages[@]}
elif grep -qw "Photon" ${rel_file}; then
  photon_packages=(
    ${common_pkgs[@]}
    build-essential
    curl-devel
    glib
    glibc-debuginfo
    python3-virtualenv
    shadow
    zlib-devel
  )

  tdnf -y upgrade --refresh
  tdnf remove -y toybox
  tdnf -y install --enablerepo=photon-debuginfo ${photon_packages[@]}
fi

