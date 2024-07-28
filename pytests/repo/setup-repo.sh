#!/bin/bash
#
# Copyright (C) 2020-2023 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU Lesser General Public License v2.1 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

if [ $# -ne 2 ]; then
    echo "Usage: $0 <repo_path> <specs_dir>"
    exit 1
fi

function fix_dir_perms()
{
  chmod 755 ${TEST_REPO_DIR}
  find ${TEST_REPO_DIR} -type d -exec chmod 0755 {} \;
  find ${TEST_REPO_DIR} -type f -exec chmod 0644 {} \;
}

## used to check return code for each command.
function check_err {
  rc=$?
  if [ $rc -ne 0 ]; then
      echo $1
      exit $rc
  fi
}

TEST_REPO_DIR=$1
if [ -d ${TEST_REPO_DIR} ]; then
    echo "Repo already exists"
    fix_dir_perms
    exit 0
fi

REPO_SRC_DIR=$2
if [ ! -d ${REPO_SRC_DIR} ]; then
    echo "specs dir does not exist"
    exit 1
fi

export GNUPGHOME=${TEST_REPO_DIR}/gnupg

BUILD_PATH=${TEST_REPO_DIR}/build
PUBLISH_PATH=${TEST_REPO_DIR}/photon-test
PUBLISH_SRC_PATH=${TEST_REPO_DIR}/photon-test-src
PUBLISH_SHA512_PATH=${TEST_REPO_DIR}/photon-test-sha512

ARCH=$(uname -m)

mkdir -p -m 755 ${BUILD_PATH}/BUILD \
    ${BUILD_PATH}/SOURCES \
    ${BUILD_PATH}/SRPMS \
    ${BUILD_PATH}/RPMS/${ARCH} \
    ${BUILD_PATH}/RPMS/noarch \
    ${TEST_REPO_DIR}/yum.repos.d \
    ${PUBLISH_PATH} \
    ${PUBLISH_SRC_PATH} \
   ${PUBLISH_SHA512_PATH} \
    ${GNUPGHOME}

#gpgkey data for unattended key generation
cat << EOF > ${TEST_REPO_DIR}/gpgkeydata
     %echo Generating a key for repogpgcheck signatures
     %no-protection
     Key-Type: default
     Subkey-Type: default
     Name-Real: tdnf test
     Name-Comment: tdnf test key
     Name-Email: tdnftest@tdnf.test
     Expire-Date: 0
     %commit
     %echo done
EOF

#generate a key non interactively. this is used in testing
#repogpgcheck plugin
gpg --batch --generate-key ${TEST_REPO_DIR}/gpgkeydata
check_err "Failed to generate gpg key."

cat << EOF > ~/.rpmmacros
%_gpg_name tdnftest@tdnf.test
%__gpg /usr/bin/gpg
EOF


for d in conflicts enhances obsoletes provides recommends requires suggests supplements ; do
    sed s/@@dep@@/$d/ < ${REPO_SRC_DIR}/tdnf-repoquery-deps.spec.in > ${BUILD_PATH}/SOURCES/tdnf-repoquery-$d.spec
done

echo building packages
for spec in ${REPO_SRC_DIR}/*.spec ${BUILD_PATH}/SOURCES/*.spec ; do
    echo "building ${spec}"
    rpmbuild  --define "_topdir ${BUILD_PATH}" \
        -r ${BUILD_PATH} -ba ${spec} 2>&1
    check_err "failed to build ${spec}"
done
rpmsign --addsign ${BUILD_PATH}/RPMS/*/*.rpm
check_err "Failed to sign built packages."
cp -r ${BUILD_PATH}/RPMS ${PUBLISH_PATH}
cp -r ${BUILD_PATH}/SRPMS ${PUBLISH_SRC_PATH}
cp -r ${BUILD_PATH}/RPMS ${PUBLISH_SHA512_PATH}

# save key to later be imported:
mkdir -p ${PUBLISH_PATH}/keys
gpg --armor --export tdnftest@tdnf.test > ${PUBLISH_PATH}/keys/pubkey.asc

createrepo ${PUBLISH_PATH}
createrepo ${PUBLISH_SRC_PATH}
createrepo -s sha512 ${PUBLISH_SHA512_PATH}

modifyrepo ${REPO_SRC_DIR}/updateinfo-1.xml ${PUBLISH_PATH}/repodata
check_err "Failed to modify repo with updateinfo-1.xml."
modifyrepo ${REPO_SRC_DIR}/updateinfo-2.xml ${PUBLISH_PATH}/repodata
check_err "Failed to modify repo with updateinfo-2.xml."

#gpg sign repomd.xml
gpg --batch --passphrase-fd 0 \
--pinentry-mode loopback \
--detach-sign --armor ${PUBLISH_PATH}/repodata/repomd.xml
check_err "Failed to gpg sign repomd.xml."

cat << EOF > ${TEST_REPO_DIR}/yum.repos.d/photon-test.repo
[photon-test]
name=basic
baseurl=http://localhost:8080/photon-test
#metalink=http://localhost:8080/photon-test/metalink
gpgkey=file:///etc/pki/rpm-gpg/VMWARE-RPM-GPG-KEY
gpgcheck=0
enabled=1
EOF

cat << EOF > ${TEST_REPO_DIR}/yum.repos.d/photon-test-sha512.repo
[photon-test-sha512]
name=basic
baseurl=http://localhost:8080/photon-test-sha512
#metalink=http://localhost:8080/photon-test-sha512/metalink
gpgkey=file:///etc/pki/rpm-gpg/VMWARE-RPM-GPG-KEY
gpgcheck=0
enabled=0
EOF

cat << EOF > ${TEST_REPO_DIR}/yum.repos.d/photon-test-src.repo
[photon-test-src]
name=basic
baseurl=http://localhost:8080/photon-test-src
gpgkey=file:///etc/pki/rpm-gpg/VMWARE-RPM-GPG-KEY
gpgcheck=0
enabled=0
EOF

cat << EOF > ${TEST_REPO_DIR}/tdnf.conf
[main]
gpgcheck=0
installonly_limit=3
clean_requirements_on_remove=true
repodir=${TEST_REPO_DIR}/yum.repos.d
cachedir=${TEST_REPO_DIR}/cache/tdnf
EOF

# size field is a dummy value - it will be ignored
cat << EOF > ${PUBLISH_PATH}/metalink
<?xml version="1.0" encoding="utf-8"?>
<metalink version="3.0" xmlns="http://www.metalinker.org/" type="dynamic" pubdate="Wed, 05 Feb 2020 08:14:56 GMT">
 <files>
  <file name="repomd.xml">
   <verification>
   </verification>
   <size>123</size>
   <resources maxconnections="1">
    <url protocol="http" type="file" location="IN" preference="100">http://localhost:8080/photon-test/repodata/repomd.xml</url>
   </resources>
  </file>
 </files>
</metalink>
EOF

cp ${REPO_SRC_DIR}/automatic.conf ${TEST_REPO_DIR}

fix_dir_perms
