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

BUILD_TYPE="release"
ARCH_INPUT="amd64"

for arg in "$@"; do
  case $arg in
    --arch=*)
      ARCH_INPUT="${arg#*=}"
      shift
      ;;
    --type=*)
      BUILD_TYPE="${arg#*=}"
      shift
      ;;
    -h|--help)
      echo "Usage: $0 [--arch=amd64|arm64] [--type=release|debug|relwithdebinfo]"
      exit 0
      ;;
    *)
      echo "[-] Unknown argument: $arg"
      echo "[?] Usage: $0 [--arch=amd64|arm64] [--type=release|debug|relwithdebinfo]"
      exit 1
      ;;
  esac
done

case "$ARCH_INPUT" in
  arm64|aarch64|armv8)
    DOCKER_PLATFORM="linux/arm64"
    ARCH_TAG="arm64"
    TARGET_ARCH="aarch64"
    ;;
  amd64|x86_64)
    DOCKER_PLATFORM="linux/amd64"
    ARCH_TAG="amd64"
    TARGET_ARCH="x86_64"
    ;;
  *)
    echo "[-] Unsupported architecture: $ARCH_INPUT (use amd64 or arm64)"
    exit 1
    ;;
esac

IMAGE_NAME="rglc-builder:${ARCH_TAG}"
CACHE_VOLUME="rglc-conan-cache-${ARCH_TAG}"

echo "=========================================="
echo " Building for architecture: ${ARCH_TAG} (${DOCKER_PLATFORM})"
echo " Target CMake build type:   ${BUILD_TYPE}"
echo " Docker image tag:          ${IMAGE_NAME}"
echo " Conan cache volume:        ${CACHE_VOLUME}"
echo "=========================================="

# Build Docker image for the target architecture
echo "[!] Building/updating Docker image '${IMAGE_NAME}'..."
docker build --platform "$DOCKER_PLATFORM" -t "$IMAGE_NAME" .

# Run build inside container
echo "[!] Launching build inside '${IMAGE_NAME}' container..."
docker run --rm \
  --platform "$DOCKER_PLATFORM" \
  -v "$PWD":/src \
  -v "${CACHE_VOLUME}":/root/.conan2 \
  "$IMAGE_NAME" \
  bash -lc "
    set -e
    cleanup() {
      chmod -R a+rwX build-* rglc.AppDir rglc-*.AppImage 2>/dev/null || true
    }
    trap cleanup EXIT
    ./scripts/build.sh --type=${BUILD_TYPE} --arch=${TARGET_ARCH} --deps --appimg
  "

echo "=========================================="
echo "[+] Build for ${ARCH_TAG} completed successfully!"
echo "[+] Look for rglc-*-${TARGET_ARCH}.AppImage in the project root."
echo "=========================================="
