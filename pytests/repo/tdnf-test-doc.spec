#
# tdnf-test-doc spec file
#
Summary:    basic install test file.
Name:       tdnf-test-doc
Version:    1.0.1
Release:    2
Vendor:     VMware, Inc.
Distribution:   Photon
License:    VMware
Url:        http://www.vmware.com
Group:      Applications/tdnftest

%description
Part of tdnf test spec. Basic install/remove/upgrade test

%prep

%build

%install
mkdir -p %_topdir/%buildroot/usr/share/doc/tdnf-test-doc
echo "hello" > %_topdir/%buildroot/usr/share/doc/tdnf-test-doc/README
mkdir -p %_topdir/%buildroot/lib/systemd/system/
cat << EOF >> %_topdir/%buildroot/lib/systemd/system/tdnf-test-doc.service
[Unit]
Description=tdnf-test-doc.service for whatprovides test.

EOF

%files
%doc /usr/share/doc/tdnf-test-doc/README
/lib/systemd/system/tdnf-test-doc.service

%changelog
*   Tue Nov 15 2016 Xiaolin Li <xiaolinl@vmware.com> 1.0.1-2
-   Add a service file for whatprovides test.
*   Tue Dec 15 2015 Priyesh Padmavilasom <ppadmavilasom@vmware.com> 1.0
-   Initial build.  First version
