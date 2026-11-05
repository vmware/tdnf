#
# Packages for dependency names longer than the solver problem buffers.
# tdnf-test-longdep-req requires a long capability that is only provided
# by tdnf-test-longdep-prov, which cannot be installed.
#
Summary:    long dependency name test package
Name:       tdnf-test-longdep-prov
Version:    1.0.0
Release:    1
Vendor:     VMware, Inc.
Distribution:   Photon
License:    VMware
Url:        http://www.vmware.com
Group:      Applications/tdnftest
BuildArch:  noarch

Provides:   tdnf-test-longdep-xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
Requires:   tdnf-test-longdep-missing

%description
Part of tdnf test spec. Solver problem reporting with long dependency names.

%prep

%build

%install

%files

%changelog
*   Thu Nov 05 2026 Oliver Kurth <oliver.kurth@broadcom.com> 1.0.0-1
-   Initial build.
