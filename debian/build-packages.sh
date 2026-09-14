#!/bin/bash
#
# Build script for creating Debian packages for tdnf
# Supports Ubuntu 22.04, 24.04, and Debian Bookworm
#

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if we're in the right directory
if [ ! -f "tdnf.spec" ] || [ ! -d "debian" ]; then
    print_error "This script must be run from the tdnf source directory"
    exit 1
fi

# Detect distribution
if [ -f /etc/os-release ]; then
    . /etc/os-release
    DISTRO="$ID"
    VERSION="$VERSION_ID"
else
    print_error "Cannot detect distribution"
    exit 1
fi

print_status "Detected distribution: $DISTRO $VERSION"

# Check if this is a supported distribution
case "$DISTRO" in
    ubuntu)
        case "$VERSION" in
            "22.04"|"24.04")
                print_status "Supported Ubuntu version: $VERSION"
                ;;
            *)
                print_warning "Untested Ubuntu version: $VERSION"
                ;;
        esac
        ;;
    debian)
        case "$VERSION" in
            "12")
                print_status "Supported Debian version: $VERSION (Bookworm)"
                ;;
            *)
                print_warning "Untested Debian version: $VERSION"
                ;;
        esac
        ;;
    *)
        print_warning "Untested distribution: $DISTRO"
        ;;
esac

# Install build dependencies
print_status "Installing build dependencies..."
sudo apt-get update
sudo apt-get install -y \
    debhelper-compat \
    cmake \
    gcc \
    make \
    libpopt-dev \
    librpm-dev \
    libssl-dev \
    libsolv-dev \
    libcurl4-openssl-dev \
    libexpat1-dev \
    libsqlite3-dev \
    zlib1g-dev \
    systemd \
    libgpgme-dev \
    pkg-config \
    devscripts \
    build-essential \
    fakeroot \
    git

# Clean previous builds
print_status "Cleaning previous builds..."
rm -rf debian/tmp debian/.debhelper debian/files

# Native packages don't need upstream tarballs
VERSION=$(dpkg-parsechangelog -S Version)
print_status "Building native package version: $VERSION"

# Build source package
print_status "Building source package..."
dpkg-buildpackage -S -us -uc

# Build binary packages
print_status "Building binary packages..."
dpkg-buildpackage -b -us -uc

print_status "Build completed successfully!"
print_status "Packages are available in the parent directory:"
ls -la ../*.deb

print_status "To install the packages, run:"
echo "  sudo dpkg -i ../*.deb"
echo "  sudo apt-get install -f  # to fix any dependency issues"
