Summary:    basic install test file.
Name:       tdnf-test3
Version:    1.0
Release:    1
Vendor:     VMware, Inc.
License:    VMware
Url:        http://www.vmware.com
Group:      Applications/tdnftest
Distribution:   Photon

%description
Spec to test install rpms from local repo

%prep
%build
%install

mkdir -p %{buildroot}%{_bindir}
echo "one" > %{buildroot}%{_bindir}/one

%files
%{_bindir}/one

%changelog
