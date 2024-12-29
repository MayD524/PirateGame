cmake_minimum_required(VERSION 3.16)

# Project configuration
project(RTSGame C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

# Debugging flags
set(CMAKE_C_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

# Source files
file(GLOB_RECURSE SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine/*.c
)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/raylib/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine/include
    /run/media/may/5e7f4d21-6691-4468-9567-61c6c67bf5d7/Github/joltc/include  # Include joltc headers
)

# Library directories
link_directories(
    /usr/local/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/build
)
# Libraries
set(LIBRARIES
    raylib
    lua
    m
    dl
    pthread
    stdc++
)

# Executable
add_executable(rts ${SOURCE_FILES})

# Link libraries
target_link_libraries(rts PRIVATE ${LIBRARIES})

# Custom post-build commands
add_custom_command(TARGET rts POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/scripts
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/src/lua ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/scripts
    COMMENT "Copying Lua scripts to build directory"
)

# Debugger integration
add_custom_target(debugger
    COMMAND gdb --args ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rts
    COMMENT "Launching GDB for debugging"
)

# Valgrind integration
add_custom_target(memcheck
    COMMAND valgrind --leak-check=full --track-origins=yes ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rts
    COMMENT "Running Valgrind for memory checks"
)
