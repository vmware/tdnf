Summary:    Repoquery Test
Name:       tdnf-conflict-file1
Version:    1.0.1
Release:    1
Vendor:     VMware, Inc.
Distribution:   Photon
License:    VMware
Url:        http://www.vmware.com
Group:      Applications/tdnftest

%description
Part of tdnf test spec. Two packages containing one identical file.

%prep

%build

%install
mkdir -p %_topdir/%buildroot/usr/lib/conflict
echo file > %_topdir/%buildroot/usr/lib/conflict/conflicting-file
%files
/usr/lib/conflict/conflicting-file

%changelog
*   Tue Jul 06 2021 Oliver Kurth <okurth@vmware.com> 1.0.1-1
-   first version
