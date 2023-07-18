#
# tdnf test spec file
#
Summary:    basic install test file.
Name:       tdnf-verbose-scripts
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
mkdir -p %_topdir/%buildroot/lib/systemd/system/
echo %_topdir/%buildroot/lib/systemd/system/%{name}.service
cat << EOF >> %_topdir/%buildroot/lib/systemd/system/%{name}.service
[Unit]
Description=%{name}.service for rpm script test.

EOF

%post
echo "echo from post"
echo "echo to stderr from post" >&2

%postun
echo "echo from postun"
echo "echo to stderr from postun" >&2

%pre
echo "echo from pre"
echo "echo to stderr from pre" >&2

%preun
echo "echo from preun"
echo "echo to stderr from preun" >&2

%files
/lib/systemd/system/%{name}.service

%changelog
*   Tue Jul 11 2023 Oliver Kurth <okurth@vmware.com>
-   test script output redirection
