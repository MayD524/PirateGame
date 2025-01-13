cmake_minimum_required(VERSION 3.16)

project(RTSGame C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

option(LOGGING_DEBUG "Enable debug mode for logging" OFF)

# Output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# Debugging flags
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra")

if (LOGGING_DEBUG)
    add_definitions(-DLOGGING_DEBUG)
endif()

# Detect if building for Emscripten
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
    message(STATUS "Configuring for Emscripten/WebAssembly.")
    # Set executable suffix to generate HTML output
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

# ------------------------------------------------------------------------------
# 1. Bring in Raylib & Lua sources via FetchContent
# ------------------------------------------------------------------------------

include(FetchContent)

# Raylib
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
)
FetchContent_MakeAvailable(raylib)

# Lua
FetchContent_Declare(
    lua
    GIT_REPOSITORY https://github.com/lua/lua.git
    GIT_TAG        v5.4.4
)
FetchContent_GetProperties(lua)
if(NOT lua_POPULATED)
    FetchContent_Populate(lua)
endif()

# ------------------------------------------------------------------------------
# 2. Build Lua as a Shared Library (DLL or .so)
#    Exclude lua.c, luac.c, and onelua.c
# ------------------------------------------------------------------------------

file(GLOB LUA_ALL_C "${lua_SOURCE_DIR}/*.c")
list(REMOVE_ITEM LUA_ALL_C
    "${lua_SOURCE_DIR}/lua.c"
    "${lua_SOURCE_DIR}/luac.c"
    "${lua_SOURCE_DIR}/onelua.c"
)

add_library(liblua SHARED ${LUA_ALL_C})
target_include_directories(liblua PUBLIC "${lua_SOURCE_DIR}")

if(WIN32 AND NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
    target_compile_definitions(liblua PRIVATE LUA_BUILD_AS_DLL)
    set_target_properties(liblua PROPERTIES OUTPUT_NAME "lua54")
else()
    target_compile_definitions(liblua PRIVATE LUA_USE_POSIX LUA_USE_DLOPEN)
    set_target_properties(liblua PROPERTIES OUTPUT_NAME "lua")
endif()

# ------------------------------------------------------------------------------
# 3. Find OpenMP, gather your own sources, build your main executable
# ------------------------------------------------------------------------------

if (NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
    find_package(OpenMP REQUIRED)
else()
    add_definitions(-DNO_OMP)
endif()

file(GLOB_RECURSE SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine/*.c
)

add_executable(rts ${SOURCE_FILES})

target_include_directories(rts PRIVATE
    ${raylib_SOURCE_DIR}/src
    ${lua_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/Engine/include
)

target_link_libraries(rts PRIVATE
    raylib
    liblua
)

if (NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
    target_link_libraries(rts PRIVATE
        OpenMP::OpenMP_C
    )
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
    target_compile_options(rts PRIVATE -msimd128)
    target_link_options(rts PRIVATE -msimd128)
endif()

# Apply Windows-specific static linking only if not building for Emscripten
if(WIN32 AND NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
    target_link_options(rts PRIVATE -static-libgcc -static-libstdc++ -static)
endif()

if(NOT WIN32)
    target_link_libraries(rts PRIVATE m dl pthread)
    target_link_options(rts PRIVATE "-Wl,-rpath,$ORIGIN")
endif()

# ------------------------------------------------------------------------------
# 4. Copy Lua scripts after building
# ------------------------------------------------------------------------------

add_custom_command(TARGET rts POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/scripts"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/src/lua"
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/scripts"
    COMMENT "Copying Lua scripts to build directory"
)

# ------------------------------------------------------------------------------
# 5. Windows-specific post-build steps (skipped for Emscripten)
# ------------------------------------------------------------------------------

if(WIN32 AND NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
    add_custom_command(TARGET rts POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:liblua>"
                "$<TARGET_FILE_DIR:rts>"
        COMMENT "Copying Lua shared library to executable directory"
    )

    # Function to copy a DLL if found
    function(copy_dll_if_found DLL_NAME SEARCH_PATH)
        find_file(${DLL_NAME}_FILE
            NAMES ${DLL_NAME}
            PATHS ${SEARCH_PATH}
            NO_DEFAULT_PATH
        )
        if(${DLL_NAME}_FILE)
            message(STATUS "Found ${DLL_NAME} at: ${${DLL_NAME}_FILE}")
            add_custom_command(TARGET rts POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${${DLL_NAME}_FILE}"
                        "$<TARGET_FILE_DIR:rts>"
                COMMENT "Copying ${DLL_NAME} to executable directory"
            )
        else()
            message(WARNING "${DLL_NAME} not found in specified path.")
        endif()
    endfunction()

    # Locate and copy required DLLs after build
    set(MINGW_BIN_PATH "/usr/x86_64-w64-mingw32/bin")
    copy_dll_if_found("libgomp-1.dll" ${MINGW_BIN_PATH})
    copy_dll_if_found("libgcc_s_seh-1.dll" ${MINGW_BIN_PATH})
    copy_dll_if_found("libwinpthread-1.dll" ${MINGW_BIN_PATH})
endif()

# ------------------------------------------------------------------------------
# 6. Utility Target: Run
# ------------------------------------------------------------------------------

add_custom_target(run
    COMMAND "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/rts"
    DEPENDS rts
    COMMENT "Running the RTSGame program..."
)

# ------------------------------------------------------------------------------
# 7. Diagnostic Messages
# ------------------------------------------------------------------------------

message(STATUS "Project Name: ${PROJECT_NAME}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Output Directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
