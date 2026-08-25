Summary:    Test package - prein ordering regression, provider A
Name:       tdnf-test-prein-dep-a
Version:    1.0
Release:    1
Vendor:     VMware, Inc.
Distribution:   Photon
License:    VMware
Url:        http://www.vmware.com
Group:      Applications/tdnftest
Provides:   tdnf-test-prein-cap
Requires:   tdnf-test-prein-base

%description
Provider A of tdnf-test-prein-cap. Requires tdnf-test-prein-base to gain
enough graph weight that transaction_order() consistently places it right
before the consumer (and after dep-b) in the install sequence.

%prep
%build
%install
%files
%changelog
*   Fri Aug 21 2026 tdnf team 1.0-1
-   Initial package
