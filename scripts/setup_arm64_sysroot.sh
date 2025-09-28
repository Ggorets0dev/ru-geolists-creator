#!/usr/bin/env bash
set -e

# ===============
# GLOBAL VARIABLES (SETTINGS)

SRIPT_TEMP_FOLDER="/tmp/RuGeolistsCreator"

UBUNTU_ARM64_SYSROOT_ARCHIVE_PATH="${SRIPT_TEMP_FOLDER}/ubuntu-base-22.04-base-arm64.tar.gz"
UBUNTU_ARM64_SYSROOT_URL="https://cdimage.ubuntu.com/ubuntu-base/releases/jammy/release/ubuntu-base-22.04-base-arm64.tar.gz"
UBUNTU_ARM64_SYSROOT_PATH="${SRIPT_TEMP_FOLDER}/ubuntu-arm64-sysroot"
# ==============

run_in_chroot() {
    # Прокидываем переменные окружения внутрь chroot
    sudo chroot "$UBUNTU_ARM64_SYSROOT_PATH" /bin/bash <<'CHROOT_EOF'
export PS1=""
echo "====== Entering arm64 sysroot (chroot) ======"
echo "PWD: $(pwd)"
echo "User: $(whoami)"

echo "[!] Installing sysroot dependencies using apt..."
chown root:root /tmp
chmod 1777 /tmp

apt-get -o Acquire::AllowInsecureRepositories=true \
    -o Acquire::AllowDowngradeToInsecureRepositories=true \
    update

apt-get install -y --allow-unauthenticated gpgv
apt-get install -y --allow-unauthenticated --no-install-recommends \
    libcurl4-openssl-dev libarchive-dev libjsoncpp-dev wget liblog4cxx12 libprotobuf23 protobuf-compiler libprotobuf-dev liblog4cxx-dev libabsl-dev

echo "====== Exiting arm64 sysroot (chroot) ======"
CHROOT_EOF
}

# Installing dependencies...
echo "[!] Installing dependencies if required..."
sudo apt install wget

# Downloading Ubuntu Base (ARM-64)
if [ ! -d "$UBUNTU_ARM64_SYSROOT_PATH" ]; then
	echo "[!] Ubuntu sysroot is not installed, downloading..."
	mkdir -p "$SRIPT_TEMP_FOLDER"
	wget "$UBUNTU_ARM64_SYSROOT_URL" -O "$UBUNTU_ARM64_SYSROOT_ARCHIVE_PATH"
	mkdir -p "$UBUNTU_ARM64_SYSROOT_PATH"
	tar -xzf "$UBUNTU_ARM64_SYSROOT_ARCHIVE_PATH" -C "$UBUNTU_ARM64_SYSROOT_PATH"
	rm -rf "$UBUNTU_ARM64_SYSROOT_ARCHIVE_PATH"
fi

echo "[+] Ubuntu Base (arm64) sysroot is installed"

# Working in sysroot
echo "[!] Checking sysroot network settings..."
if [ ! -s "${UBUNTU_ARM64_SYSROOT_PATH}/etc/resolv.conf" ]; then
    echo "[!] DNS settings are not found, fixing..."
    echo "nameserver 1.1.1.1" | sudo tee -a "${UBUNTU_ARM64_SYSROOT_PATH}/etc/resolv.conf" > /dev/null
else
    echo "[+] DNS settings found in sysroot"
fi

# Preparing sysroot for build...
run_in_chroot
