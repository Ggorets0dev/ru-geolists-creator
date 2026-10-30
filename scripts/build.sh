#!/usr/bin/env bash
set -e

# ==============================
# CHANGING PWD
# ==============================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CURRENT_DIR="$(pwd)"

if [ "$CURRENT_DIR" = "$SCRIPT_DIR" ]; then
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

# Default architecture is host machine
HOST_ARCH="$(uname -m)"
TARGET_ARCH="$HOST_ARCH"

# Parse arguments
for arg in "$@"; do
  case $arg in
    --type=*)
      BUILD_TYPE="${arg#*=}"
      shift
      ;;
    --arch=*)
      TARGET_ARCH="${arg#*=}"
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
      echo "[?] Usage: $0 --type=<debug|release|relwithdebinfo> [--arch=<amd64|arm64>] [--deps] [--appimg]"
      exit 1
      ;;
  esac
done

# Check if BUILD_TYPE was provided
if [ -z "$BUILD_TYPE" ]; then
  echo "[-] Error: --type is required"
  echo "[?] Usage: $0 --type=<debug|release|relwithdebinfo> [--arch=<amd64|arm64>] [--deps] [--appimg]"
  exit 1
fi

# Normalize architecture names
case "$TARGET_ARCH" in
  amd64|x86_64)
    TARGET_ARCH="x86_64"
    ;;
  arm64|aarch64|armv8)
    TARGET_ARCH="aarch64"
    ;;
  *)
    echo "[-] Unknown or unsupported architecture: $TARGET_ARCH"
    exit 1
    ;;
esac

case "$BUILD_TYPE" in
  release|debug|relwithdebinfo)
    ;;
  *)
    echo "[-] Unknown build type specified: $BUILD_TYPE, available: (debug, release, relwithdebinfo)"
    exit 1
    ;;
esac

# Convert Build type from lower case to normal title
case "$BUILD_TYPE" in
    "relwithdebinfo")
        BUILD_TYPE_TITLE="RelWithDebInfo"
        ;;
    "minsizerel")
        BUILD_TYPE_TITLE="MinSizeRel"
        ;;
    "debug")
        BUILD_TYPE_TITLE="Debug"
        ;;
    "release")
        BUILD_TYPE_TITLE="Release"
        ;;
    *)
        BUILD_TYPE_TITLE="$(tr '[:lower:]' '[:upper:]' <<< ${BUILD_TYPE:0:1})${BUILD_TYPE:1}"
        ;;
esac

BUILD_FOLDER_NAME="build-${BUILD_TYPE}-${TARGET_ARCH}"
TARGET_BINARY_PATH="${BUILD_FOLDER_NAME}/rglc_${SI_PROJECT_VERSION}_${TARGET_ARCH}"

echo "[?] Target architecture: ${TARGET_ARCH} (Host: ${HOST_ARCH})"
echo "[?] Selected CMake build folder: ${BUILD_FOLDER_NAME}"
echo "[?] Selected CMake build title: ${BUILD_TYPE_TITLE}"

# Setup extra CMake flags
CMAKE_BUILD_FLAGS=(-DVERSION_STRING="$SI_PROJECT_VERSION")

if [[ "$INSTALL_DEPS" == "1" ]]; then
    CONAN_PROFILE_ARGS=()
    
    # Check if cross-compiling on host
    if [[ "$HOST_ARCH" != "$TARGET_ARCH" ]]; then
        echo "[!] Cross-compilation detected: host=$HOST_ARCH -> target=$TARGET_ARCH"
        if [[ "$TARGET_ARCH" == "aarch64" ]]; then
            if [[ -f "profiles/arm64" ]]; then
                echo "[!] Using Conan profile: profiles/arm64"
                CONAN_PROFILE_ARGS=(-pr:h "profiles/arm64" -pr:b default)
            else
                echo "[-] Error: profiles/arm64 not found for cross-compiling to $TARGET_ARCH"
                exit 1
            fi
        fi
    else
        if ! conan profile show -pr default >/dev/null 2>&1; then
            echo "[!] Conan default profile not found. Detecting..."
            conan profile detect --force
        fi
        CONAN_PROFILE_ARGS=(-pr:h default -pr:b default)
    fi

    echo "[!] Installing dependencies via Conan..."
    conan install . \
        -of "$BUILD_FOLDER_NAME" \
        -s build_type="$BUILD_TYPE_TITLE" \
        "${CONAN_PROFILE_ARGS[@]}" \
        -b missing \
        -c tools.cmake.cmaketoolchain:generator=Ninja
fi

echo "[!] Configuring CMake..."
cmake -S . -B "$BUILD_FOLDER_NAME" \
    -DCMAKE_TOOLCHAIN_FILE="${BUILD_FOLDER_NAME}/build/${BUILD_TYPE_TITLE}/generators/conan_toolchain.cmake" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE_TITLE}" \
    "${CMAKE_BUILD_FLAGS[@]}" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

echo "[!] Building project with CMake..."
cmake --build "$BUILD_FOLDER_NAME" -j$(nproc)

echo "[!] Updating compile_commands.json symlink..."
ln -sf "${BUILD_FOLDER_NAME}/compile_commands.json" compile_commands.json

cp "${BUILD_FOLDER_NAME}/rglc" "$TARGET_BINARY_PATH"
echo "[+] Binary saved to: $TARGET_BINARY_PATH"

if [[ "$PREPARE_APPIMG" == "1" ]]; then
  echo "[!] Preparing AppDir..."
  ./scripts/prepare_appimg.sh rglc "$TARGET_BINARY_PATH"

  if command -v appimagetool >/dev/null 2>&1; then
    APPIMAGE_OUT="rglc-${SI_PROJECT_VERSION}-${TARGET_ARCH}.AppImage"
    echo "[!] Packaging AppImage: ${APPIMAGE_OUT}..."
    ARCH="${TARGET_ARCH}" appimagetool rglc.AppDir "$APPIMAGE_OUT"
    echo "[+] AppImage created successfully: $APPIMAGE_OUT"
  else
    echo "[*] appimagetool not found in PATH; AppDir created at rglc.AppDir."
  fi
fi
