#ifndef HABIT_H
#define HABIT_H


/*
 * A meta-header file with type-names and helper macros used in most projects.
*/

// ==================== Language Helpers ====================

// ---------- language aliases ----------
// Nicer syntax for writing/loading dll functions.

#ifdef __cplusplus
# define c_linkage extern "C"
# define c_linkage_start extern "C" {
# define c_linkage_end }
#else
# define c_linkage
# define c_linkage_start
# define c_linkage_end
#endif


// ---------- C/C++ disparitys ----------
// Compound Assignment with Braced Initializer (struct a = _type_ {m1,m2,...,}):

#if !defined(__cplusplus)
# define BICA(T) (T)
#else
# define BICA(T) T
#endif

// ---------- annotation ----------
// Aliases for quick search of overloaded keywords.

// Persistant variable declaration:
#define global static
#define local static // Function scoped.

// Translation-unit scoped function.
#define internal static



// ==================== Basic Macros ====================

// ---------- macro literals ----------

// Stringifyation:
#define STRN(s) #s
#define STRV(s) STRN(s)
#define CATN(a,b) a##b
#define CATV(a,b) CATN(a,b)
#define CAT3N(a,b,c) a##b##c
#define CAT3V(a,b,c) CAT3N(a,b,c)
#define CAT4N(a,b,c,d) a##b##c##d
#define CAT4V(a,b,c,d) CAT4N(a,b,c,d)

// ---------- macro construction helpers ----------

// Early expand inner macro - not recursive.
#define EXPAND(M) M
// Empty wrapper - e.g. avoid hanging __VA_ARGS__ [Empty(0,##__VA_ARGS__)].
#define EMPTY(...) // Give rand arg incase __VA_ARGS__ is empty.
// Late expansion.
#define LATE(...) __VA_ARGS__
// Late insert comma.
#define COMMA(...) , ##__VA_ARGS__,
// Placeholder argument.
#define NOTHING



// ==================== Program Control-Flow Macros ====================
// TODO Support for messaging on breaks.


// ---------- run-time assertions ----------

// Break method:
#if !defined(AssertBreak)
# define AssertBreak() (*(int *)0 = 0)
#endif
// Release assert:
#define ForceAssert(Expression)  if (!(Expression)) { AssertBreak(); }
// Debug assert:
#if defined(ENABLE_ASSERT)
# define Assert(Expression) ForceAssert(Expression);
#else
# define Assert(Expression)
#endif

// ---------- compiletime checks ----------
// TODO : incomplete code path etc...



// ==================== Fundamental Types ====================
// CRT SAFE HEADERS (no code generation):
// stddef.h - size_t, NULL.
// stdint.h - u/intXX_t typedefs.
// stdarg.h - va_arg, va_start, va_end, va_arg.
// intrin.h (xmmintrin etc) - intrinsic functions (rdtsc, cpuid, SSE, SSE2, etc..)

#include <stdint.h>
//#include <float.h>

//
// Unsigned integer types:
//
typedef uint8_t   U8;
typedef uint16_t  U16;
typedef uint32_t  U32;
typedef uint64_t  U64;
//
// Signed integer types:
//
typedef int8_t    S8;
typedef int16_t   S16;
typedef int32_t   S32;
typedef int64_t   S64;
//
// Boolean types:
//
typedef U8  B8;
typedef U16 B16;
typedef U32 B32;
typedef U64 B64;
//
// Memory-model dependent types (cast for pointer arithmetics):
//
typedef size_t    Imm; // array indexing.
typedef uintptr_t Umm; // unsigned pointer-precision.
typedef intptr_t  Smm; // signed pointer-precision.
//
// Floating-point types:
//
typedef float     F32; // MSB-> Sign(1b) Exponent(8b) Mantissa(23b)
typedef double    F64; // MSB-> Sign(1b) Exponent(11b) Mantissa(52b)
// Unions for initializing floats with integers.
typedef union { U32 u; F32 f; } FU32;
typedef union { U64 u; F64 f; } FU64;

//
// Function-pointer types (data and function pointers differ in some architectures):
//
typedef void(VoidFunc)(void);
// TODO : Enum & Flag Types



// ==================== Conversions ====================

// ---------- type conversions ----------

// Pointer arithmetic to force implicit conversion, should be ok in most compilers.
#define UmmFromPtr(p) ( (Umm)((U8 *)(p) - (U8 *)0) )
#define PtrFromUmm(u) ( (U8 *)0 + (u) )


// ---------- unit conversions ----------

#define KiB(x) ((Umm)(x) << 10)
#define MiB(x) ((Umm)(x) << 20)
#define GiB(x) ((Umm)(x) << 30)
#define TiB(x) ((Umm)(x) << 40)
#define PiB(x) ((Umm)(x) << 50)



// ==================== Binary Helpers ====================

// ---------- signed type binary ----------
#define X32MSbit ( (U32)0x8000'0000 )
#define X64MSbit ( (U64)0x8000'0000'0000'0000 )

// ---------- floating-point binary ----------
#define F32ExpBits ( (U32)0x7F80'0000 ) // S(0) E(111'1111'1) M(000'00...) 
#define F64ExpBits ( (U64)0x7F80'0000'0000'0000 )
#define F32ManBits ( (U32)0x007F'FFFF ) // S(0) E(000'0000'0) M(111'11...)
#define F64ManBits ( (U64)0x007F'FFFF'FFFF'FFFF )
// Binary representation of FLT_MAX/FLT_EPSILON:
#define F32TinyBin ( (U32)0x0080'0000 ) // S(0) E(000'0000'1) M(000'00...)
#define F64TinyBin ( (U64)0x0010'0000'0000'0000 )
#define F32MaxBin  ( (U32)0x7F7F'FFFF ) // S(0) E(111'1111'0) M(111'11...)
#define F64MaxBin  ( (U64)0x7FEF'FFFF'FFFF'FFFF )
// TODO : Reacognize 'not a number' (Quiet/Signaling Nans)
// TODO : Reacognize 'infinity' (positive/negative)



// ==================== Memory Helpers ====================

#define ArrayCount(a) ( sizeof(a)/sizeof(*(a)) )
#define BitCount(T)   ( sizeof(T) * 8 )

// ---------- memory alignment ----------
#define IsNotPow2(p) ( UmmFromPtr(p) & (UmmFromPtr(p)-1) )
#define IsPow2(p)    ( IsNotPow2(p) == 0 )
#define Align(p, pow2) ( UmmFromPtr(p) + ((Umm)(pow2) - 1) & ~((Umm)(pow2) - 1) )
#define Dlign(p, pow2) ( UmmFromPtr(p) & ~((Umm)(pow2) - 1) )


// ---------- memory offsets ----------
#define PtrOffs(b, p) ( UmmFromPtr( ((U8*)(p)) - (b) ) )
// Aligned offs:
#define AlignOffs(p, pow2) ( Align((p), (pow2)) - UmmFromPtr(p) )
#define DlignOffs(p, pow2) ( UmmFromPtr(p) - Dlign((p), (pow2)) )
// Aligned address of an offset:
#define EffectiveAddress(p, offs, pow2) Align(UmmFromPtr(p) + (Umm)(offs), (pow2))
// Size of an element-wise alligned array.
#define EffectiveSize(size, count, pow2) (Align((size), (pow2)) * (Umm)(count))


// ---------- memory indexing ----------
#define Member(T,m) (((T)*)0)->(m)
#define MemberOffs(T,m) UMMFromPTR(&Member(T,m))



// ==================== Integral Type Limits ====================

// ---------- integer sign bits ----------
#define I8Sign  ( (U8 )0x80                  )
#define I16Sign ( (U16)0x8000                )
#define I32Sign ( (U32)0x8000'0000           )
#define I64Sign ( (U64)0x8000'0000'0000'0000 )

// ---------- unsigned integer limits ----------
#define U8Min   ( (U8 )0                     )
#define U16Min  ( (U16)0                     )
#define U32Min  ( (U32)0                     )
#define U64Min  ( (U64)0                     )
#define U8Max   ( (U8 )0xFF                  )
#define U16Max  ( (U16)0xFFFF                )
#define U32Max  ( (U32)0xFFFF'FFFF           )
#define U64Max  ( (U64)0xFFFF'FFFF'FFFF'FFFF )

// ---------- signed integer limits ----------
#define S8Min   ( (S8 )0x80                  )
#define S16Min  ( (S16)0x8000                )
#define S32Min  ( (S32)0x8000'0000           )
#define S64Min  ( (S64)0x8000'0000'0000'0000 )
#define S8Max   ( (S8 )0x7F                  )
#define S16Max  ( (S16)0x7FFF                )
#define S32Max  ( (S32)0x7FFF'FFFF           )
#define S64Max  ( (S64)0x7FFF'FFFF'FFFF'FFFF )

// ---------- memory-model limits ----------
#define ImmMin ( (Imm)0 )
#define UmmMin ( (Umm)0 )
#define ImmMax ( (Imm)-1)
#define UmmMax ( (Umm)-1)
#define SmmMin ( (Smm)1 << (BitCount(Smm) -1) )
#define SmmMax ( ~SmmMin )

// ---------- floating-point limits ----------
#define F32Min ( (F32)-3.402823466e+38F        ) // -F32Max;
#define F64Min ( (F64)-1.7976931348623158e+308 ) // -F64Max;
#define F32Max ( (F32)3.402823466e+38F         ) // *(F32 *)&F32MaxBin
#define F64Max ( (F64)1.7976931348623158e+308  ) // *(F64 *)&F64MaxBin
// Tiny = FLT_MIN, that is the smallest representable positive value:
#define F32Tiny ( (F32)1.175494351e-38F        ) // *(F32 *)&F32TinyBin;
#define F64Tiny ( (F64)2.2250738585072014e-308 ) // *(F64 *)&F64TinyBin;
// Interval Machine Epsilon = 2^(- mantissa bit count), that is
// absolute difference between 1 and next higher representable value:
#define F32Macheps ( (F32)0.00000011920928955078125             ) // 2^(-23)
#define F64Macheps ( (F64)2.2204460492503130808472633361816e-16 ) // 2^(-52)



// ==================== Math Macros ====================

// ---------- common operations ----------
#define TRUNC(x)       ( (Smm)(x) )
#define ABS(x)         ( (x) >= 0 ? (x) : (-x) )
#define FLOOR(x)       ( (Smm)(x) += (x) >= 0 ? 0 : -1 )
#define CEIL(x)        ( (Smm)(x) += (x) >= 0 ? 1 : -1 )
#define CLAMP(x, v, y) ( (v) >= (x) ? ((v) <= (y) ? (x) : (y)) : (x) )
#define MIN(a, b)      ( (a) <= (b) ? (a) : (b) )
#define MIN3(a,b)      ( MIN( MIN( (a), (b) ), (c) ) )
#define MIN4(a,b)      ( MIN( MIN( (a), (b) ), MIN( (c), (d) ) ) )
#define MAX(a, b)      ( (a) <= (b) ? (b) : (a) )
#define MAX3(a,b)      ( MAX( MAX( (a), (b) ), (c) ) )
#define MAX4(a,b)      ( MAX( MAX( (a), (b) ), MAX( (c), (d) ) ) )


// ---------- common constants ----------

// Euler's/Napiers number (base of the natural logarithm function).
#define Euler 2.7182818284590452353602874713527;

// Golden ratio:
#define GoldenRatioSmall 0.618'0339'8874'9894'8482'0458'6834'3656
#define GoldenRatioLarge 1.618'0339'8874'9894'8482'0458'6834'3656

// Tau, Pi:
#define Pi   3.1415926535897932384626433832795
#define Pi2  1.5707963267948966192313216916398         // half pi
#define Pi4  0.7853'9816'3397'4483'0961'5660'8458'1988 // quarter pi
#define Tau  6.283185307179586476925286766559          // two pi
#define Tau2 Pi  // half tau
#define Tau4 Pi2 // quarter tau
#define Tau8 Pi4 // eighth tau



// ==================== Miscellaneous ====================

typedef U8  Ascii1ID1;
typedef U16 Ascii2ID2;
typedef U32 Ascii4ID4;
typedef U64 Ascii8ID8;
#define ASCIIID1(a)       ( (U8)(a) )
#define ASCIIID2(a,b)     ( (U16)(a) | ((U16)(b) << 8) )
#define ASCIIID4(a,b,c,d) ( (U32)(a)  | ((U32)(b) << 8) | ((U32)(c) << 16) | ((U32)(d) << 24) )
#define ASCIIID8(a,b,c,d) \
( (U64)(a)  | ((U64)(b) << 8) | ((U64)(c) << 16) | ((U64)(d) << 24) \
| (U64)(a)  | ((U64)(b) << 8) | ((U64)(c) << 16) | ((U64)(d) << 24) )



#endif // !HABIT_H
