#ifndef VECMAT_EMU_H
#define VECMAT_EMU_H

/*
 * USAGE:
 * INCLUDE THIS AFTER INCLUDING A ARCHITECTURE SPECIFIC IMPLEMENTATION
 * TO FILL ANY MISSING SIMD INTRINSICS WITH SCALAR COUNTERPARTS.
 *
 * #include "vecmat_x86.h" // 
 * #include "vecmat_emu.h" // Plug in any missing intrinsics.
 *
 * NOTE : CPU SIMD LOGISTICS
 * [] Transfer
 * 	[] Memory
 * 		[] <Load>
 * 		[] <Store>
 * 	[] Register
 * 		[] <Move>
 * 		[] <Swizzle>
 * [] Transmutation:
 * 	[] Compile-time
 * 		[] <Cast> aka type-punning.
 * 	[] Run-time
 * 		[] <Convert>
 * [] Sets
 * 	[] <Logical> bitwise operations (and[&], or[|], ...)
 * 	[] <Shift> arithmetic and logical shifting (<</<<<, >>/>>>, ...)
 * 	[] <Compare> (eq[==], ge[>], gt[>], le[<=], lt[<], neq[!=], ...)
 * [] Math
 * 	[] <Arithmetic> (add/sub, mul/div, fmad, horizontal operations, ...)
 * 	[] <Elementary Math Functions> (sqrt, ...)
 * 	[] <Special Math Functions> (abs, ceil, ...)
*/


// NOTE :
// CONSTRUCTORS
//	T_elem(e0, ..., eN) - Set each elems value individually.
//	T_same(e) - Broadcast single value to all elems.
//	T_loada/u(&) - Load elements from an array/mem.


/* ----------------------------------------------------------------------------
 *                           TLDR - USAGE
 *
 * NOTE : DO BEFORE INCLUDING THIS FILE:
 * #include "vecmat_arch.h" // Architecture specific SIMD implementation.
 * #define SIMD_NO_EMU // Disable emulation.
 *
 * NOTE : DO IN ARCHITECTURE SPECIFIC IMPLEMENTATION:
 * #define V4F32_WIDE_TYPE __m128 // Define simd specific types.
 * Replace the following operation definitions.
 *
---------------------------------------------------------------------------- */

#if 0
// OPTIONALLY REPLACE THESE:
// _wide(w) used to pass simd type as vector to allow straight forward assignment.
// Use vectors 'w' member to pass values to simd ops.
// -> V4F32_add(a, V4F32_add(a,b))
// Emulation uses braced assignment.
// -> V4F32{w}
# define V4F32_wide(/*V4F32_WIDE_TYPE*/w) EMU_V4F32_wide(w)
# define V8F32_wide(/*V8F32_WIDE_TYPE*/w) EMU_V8F32_wide(w)
# define V4F32_elem(/*float*/e0,e1,e2,e3) EMU_V4F32_elem(e0,e1,e2,e3)
# define V8F32_elem(/*float*/e0,e1,e2,e3,e4,e5,e6,e7) EMU_V8F32_elem(e0,e1,e2,e3,e4,e5,e6,e7)
// REPLACE THESE OR LEAVE FOR EMULATION:
# define V4F32_load(/*float_ptr*/m) V4F32_wide(/*put def here*/)
# define V8F32_load(/*float_ptr*/m) V4F32_wide(/*put def here*/)
# define V4F32_add(/*V4F32*/a,b) V4F32_wide(/*put def here*/)
# define V8F32_add(/*V8F32*/a,b) V4F32_wide(/*put def here*/)
# define V4F32_sub(/*V4F32*/a,b) V4F32_wide(/*put def here*/)
# define V8F32_sub(/*V8F32*/a,b) V4F32_wide(/*put def here*/)
# define V4F32_mul(/*V4F32*/a,b) V4F32_wide(/*put def here*/)
# define V8F32_mul(/*V8F32*/a,b) V4F32_wide(/*put def here*/)
# define V4F32_div(/*V4F32*/a,b) V4F32_wide(/*put def here*/)
# define V8F32_div(/*V8F32*/a,b) V4F32_wide(/*put def here*/)
#endif 


/* ----------------------------------------------------------------------------
 *                           TLDR - USAGE
---------------------------------------------------------------------------- */



/*::::::::::::::::::::::: VECTOR TYPES :::::::::::::::::::::*/

// NON-WIDE VECTOR TYPES
typedef union V2F32 {float e[2]; struct{float e0, e1;};     } V2F32;
typedef union V3F32 {float e[3]; struct{float e0, e1, e2;}; } V3F32;

//
// SIMD COMPATIBLE VECTORS
//

#if defined V4F32_WIDE_TYPE
# define V4F32_SIMD_MEMBER_w V4F32_WIDE_TYPE w;
#else
# define V4F32_SIMD_MEMBER_w
#endif
#if defined V8F32_WIDE_TYPE
# define V8F32_SIMD_MEMBER_w V8F32_WIDE_TYPE w;
#else
# define V8F32_SIMD_MEMBER_w
#endif

typedef union V4F32u 
{ V4F32_SIMD_MEMBER_w
  float e[4];
  struct{float e0,e1,e2,e3;};
  V2F32 h[2]; 
} V4F32;

typedef union V8F32 
{ V8F32_SIMD_MEMBER_w
  float e[8];
  struct{float e0,e1,e2,e3,e4,e5,e6,e7;};
  V4F32 h[2]; 
} V8F32;


// =============== non-wide floating-point vector operations ===============
#define V2F32_load(m)      BICA(V2F32) {(m)[0], (m)[1]}
#define V2F32_elem(e0, e1) BICA(V2F32) {e0, e1}
#define V2F32_add (a,b)    BICA(V2F32) {a.e0+b.e0, a.e1+b.e1}
#define V2F32_sub (a,b)    BICA(V2F32) {a.e0-b.e0, a.e1-b.e1}
#define V2F32_mul (a,b)    BICA(V2F32) {a.e0*b.e0, a.e1*b.e1}
#define V2F32_div (a,b)    BICA(V2F32) {a.e0/b.e0, a.e1/b.e1}

#define V3F32_load(m)          BICA(V3F32) {(m)[0], (m)[1], (m)[2]}
#define V3F32_elem(e0, e1, e2) BICA(V3F32) {e0, e1, e2}
#define V3F32_add (a,b)        BICA(V3F32) {a.e0+b.e0, a.e1+b.e1, a.e2+b.e2}
#define V3F32_sub (a,b)        BICA(V2F32) {a.e0-b.e0, a.e1-b.e1, a.e2-b.e2}
#define V3F32_mul (a,b)        BICA(V2F32) {a.e0*b.e0, a.e1*b.e1, a.e2*b.e2}
#define V3F32_div (a,b)        BICA(V2F32) {a.e0/b.e0, a.e1/b.e1, a.e2/b.e2}


/*::::::::::::::::::::::: SIMD IMPLEMENTATION  :::::::::::::::::::::*/
/*::::::::::::::::::::::: SIMD IMPLEMENTATION  :::::::::::::::::::::*/
/*::::::::::::::::::::::: SIMD IMPLEMENTATION  :::::::::::::::::::::*/
/*::::::::::::::::::::::: SIMD IMPLEMENTATION  :::::::::::::::::::::*/
/*::::::::::::::::::::::: SIMD IMPLEMENTATION  :::::::::::::::::::::*/





/*=============== HELPERS ===============*/

// e.g. printf("4-wide vectors elemets = "V4_REPEAT("%s"),#V4EXPAND(vector));
#define V2_EXPAND(v) v.e0, v.e1
#define V3_EXPAND(v) v.e0, v.e1, v.e2
#define V4_EXPAND(v) v.e0, v.e1, v.e2, v.e3
#define V8_EXPAND(v) v.e0, v.e1, v.e2, v.e3, v.e4, v.e5, v.e6, v.e7

#define V2_REPEAT(x) x, x
#define V3_REPEAT(x) x, x, x
#define V4_REPEAT(x) x, x, x, x
#define V8_REPEAT(x) x, x, x, x, x, x, x, x


/* ----------------------------------------------------------------------------
 *                           EMULATION
 *
 * PORTABILITY
 * Aimed at common development environments (msvx/gcc, x86/arm).
 * Assuming compiler allowes simd type construction with initilizer lists:
 * -> __m128 a = {f0,f1,f2,f3};
 *
---------------------------------------------------------------------------- */

/*::::::::::::::::::::::: EMULATION PATCHING  :::::::::::::::::::::*/

#if !defined(SIMD_NO_EMU)
# if !defined(V4F32_wide)
#   define V4F32_wide EMU_V4F32_wide
# endif
# if !defined(V8F32_wide)
#   define V8F32_wide EMU_V8F32_wide
# endif
# if !defined(V4F32_elem)
#   define V4F32_elem EMU_V4F32_elem
# endif
# if !defined(V8F32_elem)
#   define V8F32_elem EMU_V8F32_elem
# endif
# if !defined(V4F32_load)
#   define V4F32_load  EMU_V4F32_load
# endif
# if !defined(V8F32_load)
#   define V8F32_load  EMU_V8F32_load
# endif
# if !defined(V4F32_add)
#   define V4F32_add EMU_V4F32_add
# endif
# if !defined(V8F32_add)
#   define V8F32_add EMU_V8F32_add
# endif
# if !defined(V4F32_sub)
#   define V4F32_sub EMU_V4F32_sub
# endif
# if !defined(V8F32_sub)
#   define V8F32_sub EMU_V8F32_sub
# endif
# if !defined(V4F32_mul)
#   define V4F32_mul EMU_V4F32_mul
# endif
# if !defined(V8F32_mul)
#   define V8F32_mul EMU_V8F32_mul
# endif
# if !defined(V4F32_div)
#   define V4F32_div EMU_V4F32_div
# endif
# if !defined(V8F32_div)
#   define V8F32_div EMU_V8F32_div
# endif
#endif // #if !defined(SIMD_NO_EMU)



/*::::::::::::::::::::::: EMULATION DEFINITIONS  :::::::::::::::::::::*/

# define EMU_V4F32_wide(w) BICA(V4F32){w}
# define EMU_V8F32_wide(w) BICA(V8F32){w}

# define EMU_V4F32_elem(e0,e1,e2,e3) BICA(V4F32) {e0, e1, e2, e3}
# define EMU_V8F32_elem(e0,e1,e2,e3,e4,e5,e6,e7) BICA(V8F32) \
	{ V4F32_elem(e0,e1,e2,e3), V4F32_elem(e4,e5,e6,e7) }

#define EMU_V4F32_load(m) BICA(V4F32) \
	{V2_EXPAND(V2F32_load(&m[0])), V2_EXPAND(V2F32_load(&m[2]))}
#define EMU_V8F32_load(m) BICA(V8F32) \
	{V4_EXPAND(V4F32_load(&m[0])), V4_EXPAND(V4F32_load(&m[4]))}

#define EMU_V4F32_add(a,b) BICA(V4F32) \
	{a.e0+b.e0, a.e1+b.e1, a.e2+b.e2, a.e3+b.e3}
#define EMU_V8F32_add(a,b) BICA(V8F32) \
	{ V4F32_add(a.h[0], b.h[0]), V4F32_add(a.h[1], b.h[1]) }

#define EMU_V4F32_sub(a,b) BICA(V4F32) \
	{a.e0-b.e0, a.e1-b.e1, a.e2-b.e2, a.e3-b.e3}
#define EMU_V8F32_sub(a,b) BICA(V8F32) \
	{ V4F32_sub(a.h[0], b.h[0]), V4F32_sub(a.h[1], b.h[1]) }

#define EMU_V4F32_mul(a,b) BICA(V4F32) \
	{a.e0*b.e0, a.e1*b.e1, a.e2*b.e2, a.e3*b.e3}
#define EMU_V8F32_mul(a,b) BICA(V8F32) \
	{ V4F32_mul(a.h[0], b.h[0]), V4F32_mul(a.h[1], b.h[1]) }

#define EMU_V4F32_div(a,b) BICA(V4F32) \
	{a.e0/b.e0, a.e1/b.e1, a.e2/b.e2, a.e3/b.e3}
#define EMU_V8F32_div(a,b) BICA(V8F32) \
	{ V4F32_div(a.h[0], b.h[0]), V4F32_div(a.h[1], b.h[1]) }


// =============== 32-bit floating-point matrix operations ===============


#endif // !VECMAT_EMU_H
