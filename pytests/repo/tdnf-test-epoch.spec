#
# tdnf-test-epoch spec file
#
Summary:    package with a non-zero epoch, for Evr/json tests.
Name:       tdnf-test-epoch
Epoch:      1
Version:    1.0.1
Release:    1
Vendor:     VMware, Inc.
Distribution:   Photon
License:    VMware
Url:        http://www.vmware.com
Group:      Applications/tdnftest

%description
Part of tdnf test spec. Used to verify that the epoch is preserved
in the Evr field of json output (see tdnf issue #602).

%prep

%build

%install

%files

%changelog
*   Fri Sep 4 2026 Oliver Kurth <oliver.kurth@broadcom.com> 1:1.0.1-1
-   Initial build, with non-zero epoch for json Evr test.
