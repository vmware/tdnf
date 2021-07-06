Summary:    Repoquery Test
Name:       tdnf-repoquery-changelog
Version:    1.0.1
Release:    2
Vendor:     VMware, Inc.
Distribution:   Photon
License:    VMware
Url:        http://www.vmware.com
Group:      Applications/tdnftest

%description
Part of tdnf test spec. For repoquery tests, other packages will
depend on this in some way.

%prep

%build

%install
mkdir -p %_topdir/%buildroot/usr/lib/repoquery
touch %_topdir/%buildroot/usr/lib/repoquery/%name
%files
/usr/lib/repoquery/%name

%changelog
*   Tue Jul 06 2021 Oliver Kurth <okurth@vmware.com> 1.2.3-4
-   needle in a haystack
*   Wed Jan 01 2020 John Doe <jdoe@jdoe.com> 1.0.0-1
-   wrong date, needed for testing. Fedora ignores dates before
    some time 2018.
