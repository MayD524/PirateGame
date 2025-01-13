# ---------------------------------------------------------
# Emscripten Toolchain File
# ---------------------------------------------------------
# Purpose:
#   - Enable threading (pthreads)
#   - Enable SIMD
#   - Enable Asyncify (for emscripten_sleep or other async ops)
#   - Allow memory growth
#   - Provide debugging flags for development
# ---------------------------------------------------------

# 1) Identify the target system
set(CMAKE_SYSTEM_NAME Emscripten)
set(CMAKE_SYSTEM_VERSION 1)

# 2) Specify Emscripten compiler tools
set(CMAKE_C_COMPILER emcc)
set(CMAKE_CXX_COMPILER em++)
set(CMAKE_AR emar)
set(CMAKE_RANLIB emranlib)

# 3) Output file extension (.html, .js, or .wasm)
set(CMAKE_EXECUTABLE_SUFFIX ".html")

# ---------------------------------------------------------
# 4) Build Flags
# ---------------------------------------------------------

# --- PThreads ---
# Enable threading support. PTHREAD_POOL_SIZE sets the number of threads.
set(EM_PTHREAD_FLAGS "-s USE_PTHREADS=1 -s PTHREAD_POOL_SIZE=4")

# --- SIMD ---
# Enable WebAssembly SIMD instructions.
set(EM_SIMD_FLAGS "-msimd128")

# --- Memory Growth ---
# Allow the WebAssembly memory to grow at runtime.
set(EM_MEMORY_FLAGS "-s ALLOW_MEMORY_GROWTH=1")

# --- Asyncify ---
# Correctly spelled uppercase: -s ASYNCIFY
set(EM_ASYNCIFY_FLAGS "-s ASYNCIFY")

# --- Debugging / Assertions (Optional but Recommended) ---
# Enable runtime checks and safer heap operations to help catch errors.
# Also emits source maps for easier debugging in the browser.
set(EM_DEBUG_FLAGS "-s ASSERTIONS=2 -s SAFE_HEAP=1 -gsource-map")

# ---------------------------------------------------------
# 5) Consolidate all flags
# ---------------------------------------------------------
set(EM_FLAGS
    "${EM_PTHREAD_FLAGS} "
    "${EM_SIMD_FLAGS} "
    "${EM_MEMORY_FLAGS} "
    "${EM_ASYNCIFY_FLAGS} "
    "${EM_DEBUG_FLAGS}"
)

# ---------------------------------------------------------
# 6) Apply flags to compiler/linker
# ---------------------------------------------------------
set(CMAKE_C_FLAGS           "${CMAKE_C_FLAGS} ${EM_FLAGS}")
set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} ${EM_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${EM_FLAGS}")

# ---------------------------------------------------------
# 7) Additional Debug / Release Configuration
# ---------------------------------------------------------
# For Debug builds (if you do `-DCMAKE_BUILD_TYPE=Debug`)
set(CMAKE_C_FLAGS_DEBUG           "${CMAKE_C_FLAGS_DEBUG} -gsource-map")
set(CMAKE_CXX_FLAGS_DEBUG         "${CMAKE_CXX_FLAGS_DEBUG} -gsource-map")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG  "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -gsource-map")

# ---------------------------------------------------------
# 8) Threading Setup in CMake
# ---------------------------------------------------------
set(CMAKE_THREAD_LIBS_INIT "-pthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)

# ---------------------------------------------------------
# 9) Diagnostic Messages (Optional)
# ---------------------------------------------------------
message(STATUS "----------------------------------------------------")
message(STATUS "Emscripten Toolchain Configuration")
message(STATUS "C Compiler:              ${CMAKE_C_COMPILER}")
message(STATUS "C++ Compiler:            ${CMAKE_CXX_COMPILER}")
message(STATUS "C Flags:                 ${CMAKE_C_FLAGS}")
message(STATUS "C++ Flags:               ${CMAKE_CXX_FLAGS}")
message(STATUS "Linker Flags:            ${CMAKE_EXE_LINKER_FLAGS}")
message(STATUS "Build Type:              ${CMAKE_BUILD_TYPE}")
message(STATUS "----------------------------------------------------")
