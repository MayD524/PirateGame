
#pragma once
#ifndef UTIL_H
#define UTIL_H

#define true 1
#define false 0

// Some required types defined for MSVC/TinyC compiler
#if defined(_MSC_VER) || defined(__TINYC__)
    #include "propidl.h"
    #include <objbase.h>
    #include <mmreg.h>
    #include <mmsystem.h>
    #include <psapi.h>
#endif

#elif defined(__APPLE__)
#include <mach/mach.h>

#elif defined(__linux__)
#include <unistd.h>

#else
#error "Unsupported platform"
#endif
#pragma endregion


#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define FIXED_TIMESTEP 0.016666f // Approximately 60 updates per second
#define PHYSICS_TIME 0.016666666666666666667f

#include <raylib.h>
#include <extended_memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <float.h>
#include <math.h>

void s_panic(const char* caller, const char *msg);

#ifndef NDEBUG
#define panic(msg) s_panic(__func__, msg)
#else
#define panic(msg) s_panic(NULL, msg)
#endif

#ifndef FLT_EPSILON
#define FLT_EPSILON 1.19209290e-07F


#endif