#!/bin/bash
#
# Copyright (C) 2020 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU Lesser General Public License v2.1 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

if [ $# -ne 1 ]; then
	echo "Usage: $0 <repo_path>"
	exit 1
fi

if [ -d $1 ]; then
	echo "Repo already exists"
	exit 0
fi

REPO_SRC_DIR=$(dirname "$0")
TEST_REPO_DIR=$1
BUILD_PATH=${TEST_REPO_DIR}/build
PUBLISH_PATH=${TEST_REPO_DIR}/photon-test

mkdir -p ${BUILD_PATH}/BUILD \
	 ${BUILD_PATH}/SRPMS \
	 ${BUILD_PATH}/RPMS/x86_64 \
	 ${PUBLISH_PATH} \
	 ${TEST_REPO_DIR}/yum.repos.d

rpmbuild  --define "_topdir ${BUILD_PATH}" \
	  -r ${BUILD_PATH} -ba ${REPO_SRC_DIR}/*.spec > /dev/null 2>&1

cp -r ${BUILD_PATH}/RPMS ${PUBLISH_PATH}

createrepo ${PUBLISH_PATH} > /dev/null 2>&1

modifyrepo ${REPO_SRC_DIR}/updateinfo-1.xml ${PUBLISH_PATH}/repodata
modifyrepo ${REPO_SRC_DIR}/updateinfo-2.xml ${PUBLISH_PATH}/repodata

#gpgkey data for unattended key generation
GPG_PASS=`openssl rand -base64 8`
cat << EOF > ${TEST_REPO_DIR}/gpgkeydata
     %echo Generating a key for repogpgcheck signatures
     Key-Type: default
     Subkey-Type: default
     Name-Real: tdnf test
     Name-Comment: tdnf test key
     Name-Email: tdnftest@tdnf.test
     Expire-Date: 0
     Passphrase: ${GPG_PASS}
     %commit
     %echo done
EOF

#generate a key non interactively. this is used in testing
#repogpgcheck plugin
gpg --batch --generate-key ${TEST_REPO_DIR}/gpgkeydata
#gpg sign repomd.xml
echo ${GPG_PASS} | gpg --batch --passphrase-fd 0 \
--pinentry-mode loopback \
--detach-sign --armor ${PUBLISH_PATH}/repodata/repomd.xml

cat << EOF > ${TEST_REPO_DIR}/yum.repos.d/photon-test.repo
[photon-test]
name=basic
baseurl=file://${PUBLISH_PATH}
#metalink=file://${PUBLISH_PATH}
gpgkey=file:///etc/pki/rpm-gpg/VMWARE-RPM-GPG-KEY
gpgcheck=0
enabled=1
EOF

cat << EOF > ${TEST_REPO_DIR}/tdnf.conf
[main]
gpgcheck=0
installonly_limit=3
clean_requirements_on_remove=true
repodir=${TEST_REPO_DIR}/yum.repos.d
cachedir=${TEST_REPO_DIR}/cache/tdnf
EOF

cat << EOF > ${PUBLISH_PATH}/repodata/photon.metalink
<?xml version="1.0" encoding="utf-8"?>
<metalink version="3.0" xmlns="http://www.metalinker.org/" type="dynamic" pubdate="Wed, 05 Feb 2020 08:14:56 GMT">
 <files>
  <file name="repomd.xml">
   <verification>
   </verification>
   <resources maxconnections="1">
    <url protocol="ftp" type="ftp" location="IN" preference="100">file://${PUBLISH_PATH}</url>
   </resources>
  </file>
 </files>
</metalink>
EOF
