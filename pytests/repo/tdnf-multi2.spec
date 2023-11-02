#
# tdnf-test-one spec file
#
Summary:    multiinstall test
Name:       tdnf-multi
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

# files should not conflict
%install
mkdir -p %_topdir/%buildroot/usr/share/multiinstall
touch %_topdir/%buildroot/usr/share/multiinstall-%{release}

%files
/usr/share/multiinstall-%{release}

%changelog
*   Mon Jul 17 2023 Oliver Kurth <okurth@vmware.com> 1.0.1-2
-   bump
*   Mon Jul 17 2023 Oliver Kurth <okurth@vmware.com> 1.0.1-1
-   Add a service file for whatprovides test.
