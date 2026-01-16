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

to_title() {
    local input="$1"

    [ -z "$input" ] && echo "" && return

    first=$(printf '%s' "$input" | cut -c1 | tr '[:lower:]' '[:upper:]')

    rest=$(printf '%s' "$input" | cut -c2- | tr '[:upper:]' '[:lower:]')

    printf '%s%s\n' "$first" "$rest"
}

# Including global project settings
source scripts/software_info.sh

# Setup extra CMake flags
CMAKE_BUILD_FLAGS=(-DVERSION_STRING="$SI_PROJECT_VERSION")

# Parse arguments
for arg in "$@"; do
  case $arg in
    --type=*)
      BUILD_TYPE="${arg#*=}"
      shift
      ;;
    --deps)
      INSTALL_DEPS=1
      shift
      ;;
    --appimg)
      PREPARE_APPIMG=1
      shift
      ;;
    *)
      echo "[-] Unknown argument: $arg"
      echo "[?] Usage: $0 --type=<value>, --deps (optional)"
      exit 1
      ;;
  esac
done

# Check if BUILD_TYPE was provided
if [ -z "BUILD_TYPE" ]; then
  echo "[-] Error: --type is required"
  echo "[?] Usage: $0 --type=<value>"
  exit 1
fi

# Branching arch logic
case "$BUILD_TYPE" in
  release)
    # Do something for release
    ;;
  debug)
    # Do something for debug
    ;;
  *)
    # Fallback if unknown build type
    echo "[-] Unknown build type specified, available: (release, debug)"
    exit 1
    ;;
esac

BUILD_FOLDER_NAME="build-$BUILD_TYPE"
BUILD_TYPE_TITLE=$(to_title "$BUILD_TYPE")
TARGET_BINARY_PATH="${BUILD_FOLDER_NAME}/rglc_${SI_PROJECT_VERSION}"

echo "[?] Selected CMake build folder: ${BUILD_FOLDER_NAME}"
echo "[?] Selected CMake build title: ${BUILD_TYPE_TITLE}"

if [[ "$INSTALL_DEPS" == "1" ]]; then
  conan install . -of $BUILD_FOLDER_NAME -s build_type=$BUILD_TYPE_TITLE -b missing
fi

cmake -S . -B $BUILD_FOLDER_NAME -DCMAKE_TOOLCHAIN_FILE="${BUILD_FOLDER_NAME}/build/${BUILD_TYPE_TITLE}/generators/conan_toolchain.cmake" -DCMAKE_BUILD_TYPE=${BUILD_TYPE_TITLE} ${CMAKE_BUILD_FLAGS}
cmake --build $BUILD_FOLDER_NAME -j$(nproc)

mv "${BUILD_FOLDER_NAME}/rglc" $TARGET_BINARY_PATH

if [[ "$PREPARE_APPIMG" == "1" ]]; then
  ./scripts/prepare_appimg.sh rglc $TARGET_BINARY_PATH
fi
