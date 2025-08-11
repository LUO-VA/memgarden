#ifndef VECMAT_X86_H
#define VECMAT_X86_H


/* ----------------------------------------------------------------------------
 *                           SIMD
 *
 * ---------- USAGE ----------
 * #define _SSE // Enable SSE provided 4-wide floating-point instructions.
 * #include "vecmat__.cpp"
 * ...
 * float x = 1, y = 2, z = 3, w = 4;
 * float q[4] = {4,3,2,1}
 * V4F32 a = V4F32_elem(x,y,z,w);
 * V4F32 b = V4F32_load(q);
 * V4F32 c = V4F32_add(a,b);
 *
 *
 * ========== OPTIONS (define before including this file) ==========
 * ---------- instructio-set listing ----------
 *  '!' marks unsupported flags.
 *
 * [AMX] Advanced Matrix Extensions for x86.
 * !_AMX     // 2022. ---_INTEL. Accelerate AI/ML workloads.
 * [AVX-family]: Advanced Vector Extensions.
 * !_AVX10   // 2023. ---_-----. Shrink AVX512 sub-categories.
 * !_AVX512  // 2015. AMD_INTEL. 512-bit extensions.
 * _AVX2     // 2013. AMD_INTEL. 256b ext. for legacy integer ops.
 * _AVX      // 2008. AMD_INTEL. 256b ext. for legacy fp ops.
 * [FMA]: Fused-Multiply-Addition.
 * _FMA3    // 2012. AMD_INTEL.
 * !_FMA4   // 2011. AMD_-----. Support removed since Zen1 2017.
 * [SSE-family]: Streaming SIMD Extensions.
 * _SSE42    // 2008. AMD_INTEL. Deal with string operations.
 * _SSE41    // 2007. AMD_INTEL. Called 'SSE4' till 4.2 subset came.
 * _SSSE3    // 2006. AMD_INTEL. E.G. hort-add (sum elems in reg). AdvancedAudioCoding(AAC).
 * _SSE3     // 2004. AMD_INTEL. 13 instructions.
 * _SSE2     // 2000. AMD_INTEL. SSE with doubles.
 * _SSE      // 1999. AMD_INTEL. 128b floats and basic arithmetic.
 * [MMX/3DNOW]: OG wide instruction sets.
 * !_3DNOW  // 1998. AMD_-----. Only used prior to SSE.
 * !_MMX    // 1997. ---_INTEL. m64 type.
 *
---------------------------------------------------------------------------- */


#if defined(_SSE)
# define V4F32_WIDE_TYPE __m128
#endif
#if defined(_AVX)
# define V8F32_WIDE_TYPE __m256
#endif


#if 0 // LIST OF NON REPLACED OPS
# define V4F32_wide(/*V4F32_WIDE_TYPE*/w) EMU_V4F32_wide(w)
# define V8F32_wide(/*V8F32_WIDE_TYPE*/w) EMU_V8F32_wide(w)
# define V4F32_elem(/*float*/e0,e1,e2,e3) EMU_V4F32_elem(e0,e1,e2,e3)
# define V8F32_elem(/*float*/e0,e1,e2,e3,e4,e5,e6,e7) EMU_V8F32_elem(e0,e1,e2,e3,e4,e5,e6,e7)
#endif 

// ---------- sanity checks & preproc assertions ----------

// UNSUPPORTED
#if defined(_AMX)
# error ERROR (X86_SIMD_IS): UNSUPPORTED FLAG DEFINED - ERASE OFFENDER [AMX]
#endif
#if defined(_AVX10)
# error ERROR (X86_SIMD_IS): UNSUPPORTED FLAG DEFINED - ERASE OFFENDER [AVX10]
#endif
#if defined(_AVX512)
# error ERROR (X86_SIMD_IS): UNSUPPORTED FLAG DEFINED - ERASE OFFENDER [AVX512]
#endif
#if defined(_FMA4)
# error ERROR (X86_SIMD_IS): UNSUPPORTED FLAG DEFINED - ERASE OFFENDER [FMA4]
#endif
#if defined(_3DNOW)
# error ERROR (X86_SIMD_IS): UNSUPPORTED FLAG DEFINED - ERASE OFFENDER [3DNOW]
#endif
#if defined(_MMX)
# error ERROR (X86_SIMD_IS): UNSUPPORTED FLAG DEFINED - ERASE OFFENDER [MMX]
#endif

// MISMATCH
#if defined(_FMA3) && defined(_FMA4)
# error ERROR (X86_SIMD_IS): DETECTED FLAG MISMATCH - SELECT SINGLE CONTENDER [FMA3 / FMA4]
#endif
#if defined(_3DNOW) && defined(_MMX)
# error ERROR (X86_SIMD_IS): DETECTED FLAG MISMATCH - SELECT SINGLE CONTENDER [3DNOW / MMX]
#endif



/*::::::::::::::::::::::: REQUIREMENTS  :::::::::::::::::::::*/

// ---------- x86 simd intrinsic headers ----------
#include <mmintrin.h>   // MMX:  __m64   = _mm_add_pi32/epi32 () | aka {paddd mm, mm}
#include <xmmintrin.h>  // SSE:  __m128  = _mm_add_ps | aka {addps xmm, xmm}
#include <emmintrin.h>  // SSE2: __m128d = _mm_add_pd | aka {addpd xmm, xmm}
#include <pmmintrin.h>  // SSE3: __m128i = _mm_lddqu_si128 (__m128i const* mem_addr)
#include <immintrin.h>  // AVX, AVX2: __m256i = _mm256_add_epi32 (__m256i a, __m256i b);


/*::::::::::::::::::::::: SIMD IMPLEMENTATION  :::::::::::::::::::::*/

// =============== 32-bit floating-point vector operations ===============

#if defined(_SSE)
# define V4F32_load(m)  V4F32_wide(_mm_loadu_ps(m))
# define V4F32_add(a,b) V4F32_wide(_mm_add_ps((a).w, (b).w))
# define V4F32_sub(a,b) V4F32_wide(_mm_sub_ps((a).w, (b).w))
# define V4F32_mul(a,b) V4F32_wide(_mm_mul_ps((a).w, (b).w))
# define V4F32_div(a,b) V4F32_wide(_mm_div_ps((a).w, (b).w))
#endif

#if defined(_AVX)
# define V8F32_load(m)  V8F32_wide(_mm256_loadu_ps((float*)m))
# define V8F32_add(a,b) V8F32_wide(_mm256_add_ps(a.w, b.w))
# define V8F32_sub(a,b) V8F32_wide(_mm256_sub_ps(a.w, b.w))
# define V8F32_mul(a,b) V8F32_wide(_mm256_mul_ps(a.w, b.w))
# define V8F32_div(a,b) V8F32_wide(_mm256_div_ps(a.w, b.w))
#endif


// =============== 32-bit floating-point matrix operations ===============


#endif // !VECMAT_X86_H
