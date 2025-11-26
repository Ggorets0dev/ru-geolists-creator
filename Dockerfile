FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

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
    fuse \
    libfuse2 \
    autoconf automake libtool pkg-config \
    zlib1g-dev \
    libbz2-dev \
    liblzma-dev

# --------------------------------------------------------------
# Install Conan 2.x
# --------------------------------------------------------------
RUN pip3 install conan
RUN conan profile detect || true

# --------------------------------------------------------------
# Build and install bgpdump from source
# --------------------------------------------------------------
RUN git clone https://github.com/RIPE-NCC/bgpdump.git /tmp/bgpdump && \
    cd /tmp/bgpdump && \
    ls && \
    ./bootstrap.sh && \
    ./configure && \
    make && make install && \
    ldconfig

# --------------------------------------------------------------
# Install AppImage tool
# --------------------------------------------------------------
RUN wget -O /usr/local/bin/appimagetool \
    https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage && \
    chmod +x /usr/local/bin/appimagetool

# --------------------------------------------------------------
# Working directory for project
# --------------------------------------------------------------
WORKDIR /src

# --------------------------------------------------------------
# Default command — open shell
# --------------------------------------------------------------
CMD ["bash"]
