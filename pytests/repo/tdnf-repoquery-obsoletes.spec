Summary:    Repoquery Test
Name:       tdnf-repoquery-obsoletes
Version:    1.0.1
Release:    2
Vendor:     VMware, Inc.
Distribution:   Photon
License:    VMware
Url:        http://www.vmware.com
Group:      Applications/tdnftest

obsoletes: tdnf-repoquery-base

%description
Part of tdnf test spec. For repoquery tests, this package will
depend on tdnf-repoquery-base (or provide, ...)

%prep

%build

%install
mkdir -p %_topdir/%buildroot/usr/lib/repoquery
touch %_topdir/%buildroot/usr/lib/repoquery/%name
%files
/usr/lib/repoquery/%name

%changelog
*   Tue Jul 06 2021 Oliver Kurth <okurth@vmware.com> 1.0.1-2
-   first repoquery version
