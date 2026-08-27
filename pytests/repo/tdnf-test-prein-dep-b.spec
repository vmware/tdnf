Summary:    Test package - prein ordering regression, provider B
Name:       tdnf-test-prein-dep-b
Version:    1.0
Release:    1
Vendor:     VMware, Inc.
Distribution:   Photon
License:    VMware
Url:        http://www.vmware.com
Group:      Applications/tdnftest
Provides:   tdnf-test-prein-cap

%description
Provider B of tdnf-test-prein-cap. No dependencies, giving it less graph
weight than dep-a so transaction_order() consistently places it earlier.

%prep
%build
%install
%files
%changelog
*   Fri Aug 21 2026 tdnf team 1.0-1
-   Initial package
