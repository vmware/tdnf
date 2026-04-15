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

  tdnf-config edit photon-updates enabled=0
  if [ -f /etc/yum.repos.d/photon-snapshot.repo ] ; then
    tdnf-config edit photon-snapshot enabled=1
  else
     mkdir -p  /etc/tdnf/vars && echo latest > /etc/tdnf/vars/updatenumber && echo 92 > /etc/tdnf/vars/subrelease
     cat << 'EOF' > /etc/yum.repos.d/photon-snapshot.repo
[photon-snapshot]
name=VMware Photon Linux $releasever ($basearch) Snapshot for Updates
baseurl=https://packages.broadcom.com/photon/$releasever/photon_$releasever_$basearch
snapshot=https://packages.broadcom.com/photon/$releasever/photon_snapshots_$releasever_$basearch/$subrelease/snapshot-$subrelease-$updatenumber.$basearch.list
gpgkey=file:///etc/pki/rpm-gpg/VMWARE-RPM-GPG-KEY file:///etc/pki/rpm-gpg/VMWARE-RPM-GPG-KEY-4096
gpgcheck=1
enabled=1
skip_if_unavailable=1
skip_md_filelists=1
EOF
  fi

  tdnf -y upgrade --refresh
  tdnf remove -y toybox
  tdnf -y install --enablerepo=photon-debuginfo ${photon_packages[@]}
fi
