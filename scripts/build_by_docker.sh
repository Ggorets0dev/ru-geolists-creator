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

CONTAINER_NAME="rglc-amd64-dev"
IMAGE_NAME="rglc-builder"

# ==============================
# CONTAINER CHECK / CREATE
# ==============================

if ! sudo docker container inspect "$CONTAINER_NAME" >/dev/null 2>&1; then
  echo "[!] Container '$CONTAINER_NAME' not found"

  echo "[!] Building Docker image '$IMAGE_NAME'..."
  sudo docker build -t "$IMAGE_NAME" .

  echo "[!] Creating container '$CONTAINER_NAME'..."
  sudo docker run -dit \
    --name "$CONTAINER_NAME" \
    -v "$PWD":/src \
    "$IMAGE_NAME" \
    bash
else
  echo "[+] Container '$CONTAINER_NAME' exists"
fi

# ==============================
# BUILD
# ==============================

echo "[!] Launching builder container..."
sudo docker start "$CONTAINER_NAME"

echo "[!] Launching build inside builder container..."
sudo docker exec -it "$CONTAINER_NAME" bash -lc \
  "./scripts/build.sh --type=release --deps --appimg"

echo "[+] Build inside Docker is done!"

echo "[!] Stopping builder container..."
sudo docker stop "$CONTAINER_NAME"

# ==============================
# APPIMAGE
# ==============================

echo "[!] Launching appimagetool for AppImage creation..."
appimagetool rglc.AppDir

echo "[+] Release binary is done!"
