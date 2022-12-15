Summary:        Dummy obsoletes spec
Name:           tdnf-test-dummy-obsoleting
Version:        0.1
Release:        1
Vendor:         VMware, Inc.
Distribution:   Photon
License:        VMware
Url:            http://www.vmware.com
Group:          Applications/tdnftest

Provides:      tdnf-test-dummy-obsoleted
Obsoletes:      tdnf-test-dummy-obsoleted

%description
Part of tdnf test spec. This package obsoletes the -obsoleted package.

%prep

%build

%install

%files

%changelog
*  Fri Aug 07 2020 Shreenidhi Shedi <sshedi@vmware.com> 0.1-1
-  First version
