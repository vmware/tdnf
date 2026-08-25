Summary:    Test package - prein ordering base dependency
Name:       tdnf-test-prein-base
Version:    1.0
Release:    1
Vendor:     VMware, Inc.
Distribution:   Photon
License:    VMware
Url:        http://www.vmware.com
Group:      Applications/tdnftest

%description
Base package required by tdnf-test-prein-dep-a to give it more graph
weight than tdnf-test-prein-dep-b for transaction ordering tests.

%prep
%build
%install
%files
%changelog
*   Fri Aug 21 2026 tdnf team 1.0-1
-   Initial package
