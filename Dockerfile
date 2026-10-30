FROM ubuntu:22.04 AS base

ENV DEBIAN_FRONTEND=noninteractive
ENV APPIMAGE_EXTRACT_AND_RUN=1

# --------------------------------------------------------------
# Base development tools
# --------------------------------------------------------------
RUN apt update && apt install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    curl \
    wget \
    python3 \
    python3-pip \
    patchelf \
    file \
    fuse \
    libfuse2 \
    libglib2.0-0 \
    autoconf automake libtool pkg-config \
    zlib1g-dev \
    libbz2-dev \
    liblzma-dev && \
    rm -rf /var/lib/apt/lists/*

# --------------------------------------------------------------
# Install Conan 2.x
# --------------------------------------------------------------
RUN pip3 install --no-cache-dir conan
RUN conan profile detect --force || true

# --------------------------------------------------------------
# Build and install bgpdump from source
# --------------------------------------------------------------
RUN git clone https://github.com/RIPE-NCC/bgpdump.git /tmp/bgpdump && \
    cd /tmp/bgpdump && \
    ./bootstrap.sh && \
    ./configure && \
    make && make install && \
    ldconfig && \
    rm -rf /tmp/bgpdump

# --------------------------------------------------------------
# Install AppImage tool (supports both x86_64 and aarch64)
# --------------------------------------------------------------
ARG TARGETARCH
RUN set -e; \
    ARCH="${TARGETARCH:-$(dpkg --print-architecture)}"; \
    case "$ARCH" in \
        arm64|aarch64) APPIMAGE_ARCH="aarch64" ;; \
        amd64|x86_64)  APPIMAGE_ARCH="x86_64" ;; \
        *) echo "[-] Unsupported architecture for appimagetool: $ARCH" && exit 1 ;; \
    esac; \
    echo "[!] Downloading appimagetool for ${APPIMAGE_ARCH}..."; \
    curl -sSL -o /tmp/appimagetool \
        "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-${APPIMAGE_ARCH}.AppImage"; \
    chmod +x /tmp/appimagetool; \
    dd if=/dev/zero of=/tmp/appimagetool conv=notrunc bs=1 count=3 seek=8 2>/dev/null || true; \
    cd /tmp && ./appimagetool --appimage-extract >/dev/null 2>&1; \
    mv /tmp/squashfs-root /usr/local/appimagetool; \
    ln -s /usr/local/appimagetool/AppRun /usr/local/bin/appimagetool; \
    rm -f /tmp/appimagetool

# --------------------------------------------------------------
# Working directory for project
# --------------------------------------------------------------
WORKDIR /src

# --------------------------------------------------------------
# Default command — open shell
# --------------------------------------------------------------
CMD ["bash"]
