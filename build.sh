#!/bin/bash

# Build configuration
SOURCE_FILES="./src/*.c ./Engine/*.c"
OUTPUT_FILE="./build/rts"
INCLUDE_DIRS="-I./raylib/include -I./include -I./Engine/include"
LIBRARY_DIRS="-L/usr/local/lib"
LIBRARIES="-llua -lraylib -lm -ldl -lpthread"

# Ensure the build directory exists
mkdir -p ./build

# Compile the project
gcc -Os -g -o $OUTPUT_FILE $SOURCE_FILES $INCLUDE_DIRS $LIBRARY_DIRS $LIBRARIES

# Check if compilation was successful
if [ $? -ne 0 ]; then
    echo "Build failed. Please check for errors."
    exit 1
fi

# Copy Lua scripts to the build folder
mkdir -p ./build/scripts
cp ./src/lua/*.lua ./build/scripts

# Notify on success
echo "Build complete! Binary: $OUTPUT_FILE"
