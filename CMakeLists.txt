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

# Enable FetchContent module
include(FetchContent)

# Fetch raylib
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5  # Use a stable version of raylib
)
FetchContent_MakeAvailable(raylib)

# Fetch Lua
FetchContent_Declare(
    lua
    GIT_REPOSITORY https://github.com/lua/lua.git
    GIT_TAG        v5.4.4  # Use a stable version of Lua
)
FetchContent_MakeAvailable(lua)

# Find OpenMP
find_package(OpenMP REQUIRED)

# Source files
file(GLOB_RECURSE SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine/*.c
)

# Add executable
add_executable(rts ${SOURCE_FILES})

# Include directories
target_include_directories(
    rts PRIVATE
    ${raylib_SOURCE_DIR}/src
    ${lua_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine/include
)

# Link libraries
target_link_libraries(rts PRIVATE
    raylib
    lua
    m
    dl
    pthread
    OpenMP::OpenMP_C  # Link OpenMP for C
)

# Post-build: Copy Lua scripts to the build directory
add_custom_command(TARGET rts POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/scripts
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/src/lua ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/scripts
    COMMENT "Copying Lua scripts to build directory"
)

# Custom target: Run the program
add_custom_target(run
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rts
    DEPENDS rts
    COMMENT "Running the RTSGame program..."
)

# Custom target: Debug with GDB
add_custom_target(debug
    COMMAND gdb --args ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rts
    DEPENDS rts
    COMMENT "Launching GDB for debugging..."
)

# Custom target: Memory check with Valgrind
add_custom_target(memcheck
    COMMAND valgrind --leak-check=full --track-origins=yes ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rts
    DEPENDS rts
    COMMENT "Running Valgrind for memory checks..."
)
