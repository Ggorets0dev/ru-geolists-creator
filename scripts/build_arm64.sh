#!/bin/bash

# ===============
# GLOBAL VARIABLES (SETTINGS)

BUILD_FOLDER_NAME="build-arm64"
# ==============

cd ..

cmake -B "$BUILD_FOLDER_NAME" -DCMAKE_TOOLCHAIN_FILE=toolchain-arm64.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=17
cmake --build "$BUILD_FOLDER_NAME" -j$(nproc)
