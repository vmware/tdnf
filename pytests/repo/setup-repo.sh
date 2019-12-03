#!/bin/bash
#
# Copyright (C) 2015 VMware, Inc. All Rights Reserved.
#
# Licensed under the GNU Lesser General Public License v2.1 (the "License");
# you may not use this file except in compliance with the License. The terms
# of the License are located in the COPYING file of this distribution.
#

BASEDIR=$(dirname "$0")
TEST_REPO_DIR=/tmp/repo
BUILD_PATH=${TEST_REPO_DIR}/build
PUBLISH_PATH=${TEST_REPO_DIR}/photon-test

rm -rf ${TEST_REPO_DIR}

mkdir -p ${BUILD_PATH}/BUILD \
	 ${BUILD_PATH}/SRPMS \
	 ${BUILD_PATH}/RPMS/x86_64 \
	 ${PUBLISH_PATH}

rpmbuild  --define "_topdir ${BUILD_PATH}" \
	  -r ${BUILD_PATH} -ba ${BASEDIR}/*.spec > /dev/null 2>&1

cp -r ${BUILD_PATH}/RPMS ${PUBLISH_PATH}

createrepo ${PUBLISH_PATH} > /dev/null 2>&1

modifyrepo ${BASEDIR}/updateinfo-1.xml ${PUBLISH_PATH}/repodata
modifyrepo ${BASEDIR}/updateinfo-2.xml ${PUBLISH_PATH}/repodata

cat << EOF > /etc/yum.repos.d/photon-test.repo
[photon-test]
name=basic
baseurl=file://${PUBLISH_PATH}
gpgkey=file:///etc/pki/rpm-gpg/VMWARE-RPM-GPG-KEY
gpgcheck=1
enabled=1
EOF

