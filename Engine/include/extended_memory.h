#ifndef EXTENDED_MEMORY_H
#define EXTENDED_MEMORY_H

#include <stdbool.h>
#include <stddef.h>
#include <signal.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*******************************************************************************
 * Signal-based address checking (same as before, for completeness)
 ******************************************************************************/
bool is_address_good(void *addr);

/*******************************************************************************
 * Safe memory allocation utilities (same as before, for completeness)
 ******************************************************************************/
void *safe_malloc(size_t size);
void *safe_realloc(void *ptr, size_t size);
void safe_free(void **ptr);
void memory_dump(void *addr, size_t size);

#define SAFE_FREE(ptr) \
    do { \
        safe_free((void **)&(ptr)); \
    } while (0)

/*******************************************************************************
 * Struct Unpacker
 *
 * This macro generates a function that attempts to "unpack" (copy) a raw buffer
 * into a given struct, returning `true` if successful, `false` otherwise.
 *
 * Example usage:
 *
 *   typedef struct {
 *       int a;
 *       float b;
 *       char c[10];
 *   } MyStruct;
 *
 *   DEFINE_UNPACK_STRUCT(MyStruct);
 *
 *   ...
 *
 *   MyStruct s;
 *   if (unpack_MyStruct(buffer, buffer_size, &s)) {
 *       // We have a valid struct
 *   }
 ******************************************************************************/
#define DEFINE_UNPACK_STRUCT(StructType)                                     \
    bool unpack_##StructType(const void *buf, size_t buf_size, StructType *out) { \
        if (!buf || !out || buf_size < sizeof(StructType)) {                 \
            return false;                                                    \
        }                                                                    \
        /* Copy raw bytes into the struct */                                 \
        __builtin_memcpy(out, buf, sizeof(StructType));                      \
        return true;                                                         \
    }

/*******************************************************************************
 * "Rust-like" lifetime macros for automatic cleanup
 *
 * 1) AUTO_CLEANUP(fn) uses GCC/Clang's cleanup attribute to call fn at the end
 *    of the variable's scope.
 *
 * 2) AUTO_PTR(type, name, size) automatically manages memory for `name` using
 *    safe_safe_malloc, and frees it at the end of the scope.
 *
 * NOTE: These features require GCC or Clang extensions and are not portable C.
 ******************************************************************************/

#if defined(__GNUC__) || defined(__clang__)
#  define CLEANUP(fn) __attribute__((cleanup(fn)))
#else
#  warning "CLEANUP(fn) attribute not supported on this compiler. Macros will be no-ops."
#  define CLEANUP(fn)
#endif

/* Generic cleanup function for void* pointers */
static inline void generic_cleanup(void **ptr) {
    if (ptr && *ptr) {
        free(*ptr);
        *ptr = NULL;
    }
}

/**
 * AUTO_CLEANUP macro:
 *   Decorate a variable so that it calls a specified cleanup function at scope exit.
 *
 * Example:
 *   CLEANUP(generic_cleanup) char *temp = safe_safe_malloc(100);
 *   // Automatically freed when this variable goes out of scope.
 */

/**
 * AUTO_PTR macro:
 *   A convenience for automatically allocating memory and freeing it at the end
 *   of the block. Use carefully—this is purely a compile-time trick.
 *
 * Example:
 *   {
 *       AUTO_PTR(int, arr, 10 * sizeof(int));
 *       // arr is valid in this scope, and automatically freed at the end.
 *   }
 */
#define AUTO_PTR(type, name, size) \
    CLEANUP(generic_cleanup) type *name = safe_safe_malloc(size)

/*******************************************************************************
 * Example RAII-style macro with a for-loop trick
 *
 *   WITH_RESOURCE(type, var, init_expr) {
 *       // use var
 *   }
 *
 *  At the end of the block, var is automatically freed.
 ******************************************************************************/
#define WITH_RESOURCE(type, var, init_expr)                                     \
    for (bool _keep_going = true; _keep_going; )                                \
        for (type *var = (type *)safe_safe_malloc(init_expr); var != NULL; safe_free((void **)&var), _keep_going = false)

/*******************************************************************************
 * Additional prototypes for any new features can go here.
 ******************************************************************************/

#endif // EXTENDED_MEMORY_H
