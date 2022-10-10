#
# tdnf-test-one spec file
#
Summary:    basic install test file.
Name:       tdnf-test-noarch
Version:    1.0.1
Release:    1
Vendor:     VMware, Inc.
Distribution:   Photon
License:    VMware
Url:        http://www.vmware.com
Group:      Applications/tdnftest
BuildArch:  noarch

%description
Part of tdnf test spec. Basic install/remove/upgrade test

%prep

%build

%install
mkdir -p %_topdir/%buildroot/lib/systemd/system/
cat << EOF >> %_topdir/%buildroot/lib/systemd/system/%{name}.service
[Unit]
Description=%{name} for arch related tests

EOF

%files
/lib/systemd/system/%{name}.service

%changelog
*   Mon Oct 10 2022 Oliver Kurth <okurth@vmware.com> 1.0.1
-   Initial build.  First version
