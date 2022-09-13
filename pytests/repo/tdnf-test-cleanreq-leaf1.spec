#
# tdnf-test-one spec file
#
Summary:    basic install test file.
Name:       tdnf-test-cleanreq-leaf1
Version:    1.0.1
Release:    3
Vendor:     VMware, Inc.
Distribution:   Photon
License:    VMware
Url:        http://www.vmware.com
Group:      Applications/tdnftest
Requires:   tdnf-test-cleanreq-required

%description
Part of tdnf test spec. Basic install/remove/upgrade test

%prep

%build

%install
mkdir -p %_topdir/%buildroot/lib/cleanreq/
touch %_topdir/%buildroot/lib/cleanreq/%name

%files
/lib/cleanreq/%name

%changelog
*   Thu Nov 04 2021 Oliver Kurth <okurth@vmware.com> 1.0.1-3
-   clean req test
*   Tue Nov 15 2016 Xiaolin Li <xiaolinl@vmware.com> 1.0.1-2
-   Add a service file for whatprovides test.
*   Tue Dec 15 2015 Priyesh Padmavilasom <ppadmavilasom@vmware.com> 1.0
-   Initial build.  First version
