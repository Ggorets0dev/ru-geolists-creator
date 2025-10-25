#!/usr/bin/env bash
set -e

# ==============================
# CHANGING PWD
# ==============================

# Absolute path to directory with script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

CURRENT_DIR="$(pwd)"

if [ "$CURRENT_DIR" = "$SCRIPT_DIR" ]; then
  # Entering project's root
  cd ..
fi
# ==============================

# Including global project settings
source scripts/software_info.sh

# Parse arguments
for arg in "$@"; do
  case $arg in
    --arch=*)
      ARCH="${arg#*=}"
      shift
      ;;
    *)
      echo "[-] Unknown argument: $arg"
      echo "[?] Usage: $0 --arch=<value>"
      exit 1
      ;;
  esac
done

# Check if ARCH was provided
if [ -z "$ARCH" ]; then
  echo "[-] Error: --arch is required"
  echo "[?] Usage: $0 --arch=<value>"
  exit 1
fi

# ===============
# GLOBAL VARIABLES (SETTINGS)

BUILD_FOLDER_NAME="build-$ARCH"
  
TARGET_BINARY_PATH="${BUILD_FOLDER_NAME}/rglc_${SI_PROJECT_VERSION}_$ARCH"
LOG4CXX_CONFIG_PATH="log4cxx.properties"
POSTINST_SCRIPT_PATH="scripts/postinst"

DEB_PACKAGE_DIR="rglc_deb"
# ==============

# Branching arch logic
case "$ARCH" in
  amd64)
    # Do something for amd64
    echo "[!] Selected arch: amd64"
    # Extra options for AMD-64 arch...
    ;;
  arm64)
    # Do something for arm64
    echo "[!] Selected arch: arm64"
    # Extra options for ARM-64 arch...
    ;;
  *)
    # Fallback if unknown arch
    echo "[-] Unknown architecture specified, available: (amd64, arm64)"
    exit 1
    ;;
esac

# Creating package structure
mkdir -p "${DEB_PACKAGE_DIR}/DEBIAN"
mkdir -p "${DEB_PACKAGE_DIR}/usr/bin"
mkdir -p "${DEB_PACKAGE_DIR}/usr/share/ru-geolists-creator/config"

# Copy just built binary
cp "$TARGET_BINARY_PATH" "${DEB_PACKAGE_DIR}/usr/bin/rglc"

# Copy log4cxx configuration
cp "$LOG4CXX_CONFIG_PATH" "${DEB_PACKAGE_DIR}/usr/share/ru-geolists-creator/config"

# Copy postinst script
cp "$POSTINST_SCRIPT_PATH" "${DEB_PACKAGE_DIR}/DEBIAN"

# Creating control file for package
cat > "${DEB_PACKAGE_DIR}/DEBIAN/control" << EOF
Package: ru-geolists-creator
Version: ${SI_PROJECT_VERSION}
Section: utils
Priority: optional
Architecture: ${ARCH}
Maintainer: Ggorets0dev <nikgorets4work@gmail.com>
Depends: libcurl4, libarchive13, libjsoncpp25, libprotobuf23, liblog4cxx12
Description: RuGeolistsCreator - Software for automatic assembly of geoip and geosite files for VPN server XRay. Software is focused on blocking in the Russian Federation
EOF

# Building package using dpkg
dpkg-deb --build "$DEB_PACKAGE_DIR"
mv "${DEB_PACKAGE_DIR}.deb" "rglc_${SI_PROJECT_VERSION}_${ARCH}.deb"

echo "[+] DEB package build completed: rglc_${SI_PROJECT_VERSION}_${ARCH}.deb"
