#!/bin/bash

# Configuration
PROJECT_NAME="RTS_Hell"   # Replace with your project name
BUILD_DIR="build_$1"         # Base build directory
EMSDK_PATH="/run/media/may/5e7f4d21-6691-4468-9567-61c6c67bf5d7/Github/emsdk"  # Path to Emscripten SDK (adjust as needed)

# Clean and create build directory
clean_build_dir() {
    echo "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
    mkdir -p "$BUILD_DIR"
}

# Build for Linux/macOS
build_native() {
    echo "Building for $1..."
    clean_build_dir
    cd "$BUILD_DIR" || exit
    cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DLOGGING_DEBUG=ON ..
    make -j$(nproc) || { echo "Build failed."; exit 1; }
    echo "Build for $1 completed successfully."
    cd ..
}

# Build for Windows (MinGW)
build_windows() {
    echo "Building for Windows..."
    clean_build_dir
    cd "$BUILD_DIR" || exit
    cmake -DCMAKE_TOOLCHAIN_FILE=windows_toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DLOGGING_DEBUG=ON ..
    make -j$(nproc) || { echo "Build failed."; exit 1; }
    echo "Build for Windows completed successfully."
    cd ..
}

# Build for Web (Emscripten)
build_web() {
    if [ ! -d "$EMSDK_PATH" ]; then
        echo "Emscripten SDK not found at $EMSDK_PATH. Please install it."
        exit 1
    fi

    echo "Loading Emscripten environment..."
    source "$EMSDK_PATH/emsdk_env.sh"

    echo "Building for Web..."
    clean_build_dir
    cd "$BUILD_DIR" || exit
    emcmake cmake -DCMAKE_BUILD_TYPE=Release -DLOGGING_DEBUG=ON -DPLATFORM=Web ..
    emmake make || { echo "Build failed."; exit 1; }
    echo "Build for Web completed successfully."
    echo "Output files: $BUILD_DIR/$PROJECT_NAME.html, $PROJECT_NAME.js, $PROJECT_NAME.wasm"
    cd ..
}

# Detect platform and build
case "$1" in
    linux)
        build_native "Linux"
        ;;
    mac)
        build_native "macOS"
        ;;
    windows)
        build_windows
        ;;
    web)
        build_web
        ;;
    clean)
        clean_build_dir
        echo "Cleaned build directory."
        ;;
    *)
        echo "Usage: $0 {linux|mac|windows|web|clean}"
        exit 1
        ;;
esac
