#!/bin/bash
#
# Copyright (C) 2020-2021 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU Lesser General Public License v2.1 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

if [ $# -ne 1 ]; then
	echo "Usage: $0 <repo_path>"
	exit 1
fi

fix_dir_perms()
{
  chmod 755 ${TEST_REPO_DIR}
  find ${TEST_REPO_DIR} -type d -exec chmod 0755 {} \;
  find ${TEST_REPO_DIR} -type f -exec chmod 0644 {} \;
}

TEST_REPO_DIR=$1
if [ -d $1 ]; then
	echo "Repo already exists"
  fix_dir_perms
	exit 0
fi

REPO_SRC_DIR=$(dirname "$0")
BUILD_PATH=${TEST_REPO_DIR}/build
PUBLISH_PATH=${TEST_REPO_DIR}/photon-test

ARCH=$(uname -m)

mkdir -p -m 755 ${BUILD_PATH}/BUILD \
	 ${BUILD_PATH}/SRPMS \
	 ${BUILD_PATH}/RPMS/${ARCH} \
	 ${BUILD_PATH}/RPMS/noarch \
	 ${PUBLISH_PATH} \
	 ${TEST_REPO_DIR}/yum.repos.d

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

cat << EOF > ~/.rpmmacros
%_gpg_name tdnftest@tdnf.test
%__gpg /usr/bin/gpg
EOF

for d in conflicts enhances obsoletes provides recommends requires suggests supplements ; do
    sed s/@@dep@@/$d/ < ${REPO_SRC_DIR}/tdnf-repoquery-deps.spec.in > ${REPO_SRC_DIR}/tdnf-repoquery-$d.spec
done

echo building packages
rpmbuild  --define "_topdir ${BUILD_PATH}" \
	  -r ${BUILD_PATH} -ba ${REPO_SRC_DIR}/*.spec
rpmsign --addsign ${BUILD_PATH}/RPMS/*/*.rpm
cp -r ${BUILD_PATH}/RPMS ${PUBLISH_PATH}

# save key to later be imported:
mkdir -p ${PUBLISH_PATH}/keys
gpg --armor --export tdnftest@tdnf.test > ${PUBLISH_PATH}/keys/pubkey.asc

createrepo ${PUBLISH_PATH} > /dev/null 2>&1

modifyrepo ${REPO_SRC_DIR}/updateinfo-1.xml ${PUBLISH_PATH}/repodata
modifyrepo ${REPO_SRC_DIR}/updateinfo-2.xml ${PUBLISH_PATH}/repodata

#gpg sign repomd.xml
gpg --batch --passphrase-fd 0 \
--pinentry-mode loopback \
--detach-sign --armor ${PUBLISH_PATH}/repodata/repomd.xml

cat << EOF > ${TEST_REPO_DIR}/yum.repos.d/photon-test.repo
[photon-test]
name=basic
baseurl=http://localhost:8080/photon-test
#metalink=http://localhost:8080/photon-test/metalink
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

cat << EOF > ${PUBLISH_PATH}/metalink
<?xml version="1.0" encoding="utf-8"?>
<metalink version="3.0" xmlns="http://www.metalinker.org/" type="dynamic" pubdate="Wed, 05 Feb 2020 08:14:56 GMT">
 <files>
  <file name="repomd.xml">
   <verification>
   </verification>
   <resources maxconnections="1">
    <url protocol="http" type="file" location="IN" preference="100">http://localhost:8080/photon-test/repodata/repomd.xml</url>
   </resources>
  </file>
 </files>
</metalink>
EOF

fix_dir_perms
