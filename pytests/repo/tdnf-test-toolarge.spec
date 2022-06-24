#
# tdnf-test-toolarge spec file
#
Summary:    creates a fake random file that is too large for cache dir.
Name:       tdnf-test-toolarge
Version:    1.0.1
Release:    1
Vendor:     VMware, Inc.
Distribution:   Photon
License:    VMware
Url:        http://www.vmware.com
Group:      Applications/tdnftest

%description
Part of tdnf test spec. Creates a large file to intentionally trigger an
out of disk space error.

%prep

%build

%install
mkdir -p %_topdir/%buildroot/lib/toolarge/
touch %_topdir/%buildroot/lib/toolarge/largedata.txt
dd if=/dev/urandom of=%_topdir/%buildroot/lib/toolarge/largedata.txt bs=1M count=1

%files
/lib/toolarge/largedata.txt

%changelog
*   Thu May 23 2022 Preston Narchetti <pnarchetti@vmware.com> 1.0
-   Initial build.  First version

