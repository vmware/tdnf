Summary:    Test package - prein ordering regression, consumer
Name:       tdnf-test-prein-consumer
Version:    1.0
Release:    1
Vendor:     VMware, Inc.
Distribution:   Photon
License:    VMware
Url:        http://www.vmware.com
Group:      Applications/tdnftest
Requires(pre): tdnf-test-prein-cap

%description
Consumer of tdnf-test-prein-cap via Requires(pre). The %pre scriptlet
ends with a never-taken if branch to verify POSIX sh exit-code semantics.

%pre
if [ "%{name}" = "not-this-name" ]; then
    echo "unreachable"
fi

%prep
%build
%install
%files
%changelog
*   Fri Aug 21 2026 tdnf team 1.0-1
-   Initial package
