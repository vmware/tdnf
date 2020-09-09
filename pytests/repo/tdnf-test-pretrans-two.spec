#
# tdnf-test-pretrans-2 spec file
#
Summary:    Test Requires(pretrans) dependency
Name:       tdnf-test-pretrans-two
Version:    1.0
Release:    1
Vendor:     VMware, Inc.
Distribution:   Photon
License:    VMware
Url:        http://www.vmware.com
Group:      Applications/tdnftest
Requires(pretrans): tdnf-dummy-pretrans < 1.0-2

%description
Test Requires(pretrans) dependency

%prep

%build

%install

%files

%changelog
*  Tue Sep 08 2020 Keerthana K <keerthanak@vmware.com> 1.0-1
-  First version
