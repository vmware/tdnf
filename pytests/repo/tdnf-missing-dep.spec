#
# tdnf-missing spec file
#
Summary:    basic install test file.
Name:       tdnf-missing-dep
Version:    1.0.1
Release:    2
Vendor:     VMware, Inc.
Distribution:   Photon
License:    VMware
Url:        http://www.vmware.com
Group:      Applications/tdnftest

# requires something we do not have
Requires: missing

%description
Part of tdnf test spec. Basic install/remove/upgrade test

%prep

%build

%install
mkdir -p %_topdir/%buildroot/lib/systemd/system/
cat << EOF >> %_topdir/%buildroot/lib/systemd/system/%name.service
[Unit]
Description=%name.service for whatprovides test.

EOF

%files
/lib/systemd/system/%name.service

%changelog
*   Fri Sep 16 Oliver Kurth <okurth@vmware.com> 1.0
-   Initial build.  First version
