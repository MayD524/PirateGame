#ifndef SIMD_COMPAT_H
#define SIMD_COMPAT_H

#ifdef __EMSCRIPTEN__

#ifndef __wasm_simd128__
#error "SIMD128 not enabled!
#endif

#include <wasm_simd128.h>

// Define the SIMD type for WebAssembly
typedef v128_t simd_type;

// Equivalent to `_mm_set1_ps`
static inline simd_type simd_set1_ps(float x) {
    return wasm_f32x4_splat(x);
}

static inline simd_type simd_set_ps1(float x) {
    return wasm_f32x4_splat(x);
}

static inline simd_type simd_set_ps(float e3, float e2, float e1, float e0) {
    return wasm_f32x4_make(e0, e1, e2, e3); // Reverse the order of arguments
}

// Equivalent to `_mm_setr_ps`
static inline simd_type simd_setr_ps(float e0, float e1, float e2, float e3) {
    return wasm_f32x4_make(e0, e1, e2, e3);
}

// Equivalent to `_mm_add_ps`
static inline simd_type simd_add_ps(simd_type a, simd_type b) {
    return wasm_f32x4_add(a, b);
}

// Equivalent to `_mm_sub_ps`
static inline simd_type simd_sub_ps(simd_type a, simd_type b) {
    return wasm_f32x4_sub(a, b);
}

// Equivalent to `_mm_mul_ps`
static inline simd_type simd_mul_ps(simd_type a, simd_type b) {
    return wasm_f32x4_mul(a, b);
}

// Equivalent to `_mm_cmpgt_ps`
static inline simd_type simd_cmpgt_ps(simd_type a, simd_type b) {
    return wasm_f32x4_gt(a, b);
}

// Equivalent to `_mm_cmple_ps`
static inline simd_type simd_cmple_ps(simd_type a, simd_type b) {
    return wasm_f32x4_le(a, b);
}

// Equivalent to `_mm_or_ps`
static inline simd_type simd_or_ps(simd_type a, simd_type b) {
    return wasm_v128_or(a, b);
}

// Equivalent to `_mm_movemask_ps`
static inline int simd_movemask_ps(simd_type a) {
    return wasm_i32x4_bitmask(a); // Produces a 4-bit mask
}

#else
#include <immintrin.h>

// Define the SIMD type for x86
typedef __m128 simd_type;

// Map SSE intrinsics directly for x86
#define simd_set_ps1 _mm_set_ps1
#define simd_set1_ps _mm_set1_ps
#define simd_set_ps  _mm_set_ps
#define simd_setr_ps _mm_setr_ps
#define simd_add_ps _mm_add_ps
#define simd_sub_ps _mm_sub_ps
#define simd_mul_ps _mm_mul_ps
#define simd_cmpgt_ps _mm_cmpgt_ps
#define simd_cmple_ps _mm_cmple_ps
#define simd_or_ps _mm_or_ps
#define simd_movemask_ps _mm_movemask_ps

#endif // __EMSCRIPTEN__

#endif // SIMD_COMPAT_H
