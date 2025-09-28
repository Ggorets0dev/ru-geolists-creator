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

# ===============
# GLOBAL VARIABLES (SETTINGS)

CMAKE_BUILD_FLAGS=(-DVERSION_STRING="$SI_PROJECT_VERSION" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=17)
# ==============

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

# Branching arch logic
case "$ARCH" in
  amd64)
    # Do something for amd64
    echo "[!] Selected arch: amd64"
    # No extra flags for CMake needed
    ;;
  arm64)
    # Do something for arm64
    echo "[!] Selected arch: arm64"
    CMAKE_BUILD_FLAGS+=("-DCMAKE_TOOLCHAIN_FILE=toolchain-arm64.cmake")
    ;;
  *)
    # Fallback if unknown arch
    echo "[-] Unknown architecture specified, available: (amd64, arm64)"
    exit 1
    ;;
esac
    
BUILD_FOLDER_NAME="build-$ARCH"  
TARGET_BINARY_PATH="${BUILD_FOLDER_NAME}/rglc_${SI_PROJECT_VERSION}_$ARCH"   
   
echo "[!] Using CMake flags: ${CMAKE_BUILD_FLAGS[@]}"   
   
cmake -B "$BUILD_FOLDER_NAME" "${CMAKE_BUILD_FLAGS[@]}"
cmake --build "$BUILD_FOLDER_NAME" -j$(nproc)
    
mv "${BUILD_FOLDER_NAME}/RuGeolistsCreator" "$TARGET_BINARY_PATH"

echo "[+] Build is completed, binary: ${TARGET_BINARY_PATH}"
