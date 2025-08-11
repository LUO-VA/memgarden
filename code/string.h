#ifndef STRING_H
#define STRING_H

/******************************************************************************
 * STRING.H
 *
 * DESCRIPTION:
 * Basic string operations.
 * All strings treated as 8bit ascii, utf en-/decoding handled separately.
 * String memory is treated as constant, to which strings point.
 *
******************************************************************************/

// TODO: UTF en-/decoding - 8(linux) / 16(windows) / 32(straight mapping)


typedef struct {
  U8 *data;
  Umm size;
} String8;



// ==================== Primitive String Operations ====================

// ---------- Helpers ----------

// Get end of string ptr.
#define Str8_eos(s) ( (s).data + (s).size )

// Get index at count end of string.
#define Str8_rear(s, n) ( (s).size - (n) )

// Get index from ptr.
#define Str8_offs(s, p) PtrOffs((s).data, (p))

// Get ptr from index.
#define Str8_addr(s, i) ( (s).data + (i) )

// Returns 0 if no nt present at end of string.
#define Str8_is_nt(s) ( *((s).data + (s).size - 1) == 0 )


// ---------- String Construction ----------

c_linkage String8 Str8_make(void *data, Umm size);

// Empty struct:
#define Str8_zero() Str8_make(0, 0)

// Remove null-terminator:
#define Str8_lit(literal) Str8_make(literal, sizeof(literal) - 1)
c_linkage String8 Str8_from_cstr(void *cstr);

// Keep null-terminator:
#define Str8_lit_nt(literal) Str8_make(literal, sizeof(literal))
c_linkage String8 Str8_from_cstr_nt(void *cstr);

// Create null-terminated (doesn't check if nt already exists):
c_linkage String8 Str8_to_cstr(String8 s, void *cstr_dst);


////////////////////////
// Sub-String Construction.

// Grab head:
c_linkage String8 Str8_yank_i(String8 s,   Umm n);
c_linkage String8 Str8_yank_p(String8 s, void *p);
// Grab tail:
c_linkage String8 Str8_trim_i(String8 s,   Umm n);
c_linkage String8 Str8_trim_p(String8 s, void *p);
// Grab body aka range (Begin included, End not included):
c_linkage String8 Str8_clip_i(String8 s,   Umm b,   Umm e);
c_linkage String8 Str8_clip_p(String8 s, void *b, void *e);
#define Str8_clip_i_n(s, i, n) Str8_clip_i((s), (i), (i)+(n))
#define Str8_clip_i_p(s, i, p) Str8_clip_i((s), (i), Str8_offs((s), (p)))
#define Str8_clip_p_n(s, p, n) Str8_clip_p((s), (p), (U8*)(p) + (n))
#define Str8_clip_p_i(s, p, i) Str8_clip_p((s), (p), Str8_addr((s), (i)))
// Grab via sub-string (included/excluded):
#define Str8_subjoin(s, u) Str8_yank_p((s), Str8_eos((u)))
#define Str8_disjoin(s, u) Str8_yank_p((s), (u).data)

// Reverse (from end of string to start):
#define Str8_yank_i_r(s, n) Str8_trim_i((s), Str8_rear((s), (n)))
#define Str8_yank_p_r(s, p) Str8_trim_p((s), (p))
#define Str8_trim_i_r(s, n) Str8_yank_i((s), Str8_rear((s), (n)))
#define Str8_trim_p_r(s, p) Str8_yank_p((s), (p))
#define Str8_clip_i_r(s, e, b) Str8_clip_i((s), (b), (e))
#define Str8_clip_p_r(s, e, b) Str8_clip_p((s), (b), (e))
#define Str8_clip_i_n_r(s, i, n) Str8_clip_i((s), (i) - (n),           (i))
#define Str8_clip_i_p_r(s, i, p) Str8_clip_i((s), Str8_offs((s), (p)), (i))
#define Str8_clip_p_n_r(s, p, n) Str8_clip_p((s), (U8*)(p) - (n),      (p))
#define Str8_clip_p_i_r(s, p, i) Str8_clip_p((s), Str8_addr((s),(i)),  (p))
#define Str8_subjoin_r(s, u) Str8_trim_p((s), (u).data)
#define Str8_disjoin_r(s, u) Str8_trim_p((s), Str8_eos((u)))


////////////////////////
// Sub-String Combination:

// Melding:
// Form string from a prefix, append, and sub-strings with a separator between each.
// Separator is never put after/before prefix/append respectively.
// Any may be ommitted by giving a zero string.
// [@Unsafe - no checking done for null-pointers ect...]

// Returns total size for meld.
c_linkage Umm Str8_meld_size(String8 *sub_ptrs[], U8 sub_cnt, Umm separator_size, Umm prefix_size, Umm append_size, B8 is_nt);
// Returns idx given string in meld array, -1 on failure.
c_linkage U8 Str8_meld_find(String8 *sub_ptrs[], U8 sub_cnt, String8 *target);
// Returns new size for meld after swapping a string via index, -1 on failure.
c_linkage Umm Str8_meld_swap(String8 *sub_ptrs[], Umm old_meld_size, String8 *replacement, U8 idx);
// Execute meld.
c_linkage String8 Str8_meld(void *dst, String8 *sub_ptrs[], U8 sub_cnt, String8 separator, String8 prefix, String8 append, B8 is_nt);



// ---------- String Manipulation Operations ----------
// TODO : to_lower-/upprecase


// ---------- String Search Operations ----------
// Find is case sensitive.
// To ignore case, turn search and target strings to upper/lowercase.

c_linkage String8 Str8_find(String8 field, String8 grain, String8 *opt_remainder, String8 *opt_preceider);
c_linkage String8 Str8_find_r(String8 field, String8 grain, String8 *opt_remainder, String8 *opt_preceider);
// Result is first longest match found and if none found then size = 0,
// but data points to start of iteration in search area string.
// Optional:
//   _remainder receives non-searched string.
//   _preceider receives searched string prior to match.
//   Both set data ptr to start/end even if their size is 0.
//   When no match found then one of these equals given search area string.



// ==================== String List Operations ====================
// TODO :



#endif // !STRING_H
