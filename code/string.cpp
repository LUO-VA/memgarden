//#include "string.h"


// ==================== Primitive String Operations ====================

// ---------- Helpers ----------

c_linkage B8 String8_is_nt(String8 s)
{
  B8 result = 0;
  if ( *(s.data + s.size - 1) == 0 )
  {
    result = 1;
  }
  return(result);
}


// ---------- String Construction ----------

c_linkage String8
Str8_make(void *data, Umm size)
{
  String8 result = {(U8*)data, size};
  return(result);
}

c_linkage String8
Str8_from_cstr(void *cstr)
{
  U8 *eos = (U8*)cstr;
  while (*eos != 0) { eos += 1; }
  return Str8_make(cstr, UmmFromPtr(eos - (U8*)cstr));
}

c_linkage String8
Str8_from_cstr_nt(void *cstr)
{
  U8 *eos = (U8*)cstr;
  while (*eos != 0) { eos += 1; }
  return Str8_make(cstr, UmmFromPtr(eos + 1 - (U8*)cstr));
}

c_linkage String8
Str8_to_cstr(String8 s, void *cstr_dst)
{
  OSMemory_copy(cstr_dst, s.data, s.size);
  *((U8*)cstr_dst + s.size) = '\0';
  return Str8_make(cstr_dst, s.size + 1);
}


////////////////////////
// Sub-String Construction.

c_linkage String8
Str8_yank_i(String8 s, Umm n)
{
  Assert(n <= s.size);
  return Str8_make(s.data, n);
}

c_linkage String8
Str8_yank_p(String8 s, void *p)
{
  U8 *l = (U8*)p;
  Assert(s.data <= l && l <= s.data + s.size);
  return Str8_make(s.data, UmmFromPtr(l - s.data));
}


c_linkage String8
Str8_trim_i(String8 s, Umm n)
{
  Assert(n <= s.size);
  return Str8_make(s.data + n, s.size - n);
}

c_linkage String8
Str8_trim_p(String8 s, void *p)
{
  U8 *f = (U8*)p;
  Assert(s.data <= f && f <= s.data + s.size);
  return Str8_make(f, s.size - UmmFromPtr(f - s.data));
}


c_linkage String8
Str8_clip_i(String8 s, Umm b, Umm e)
{
  Assert(b <= e && e <= s.size);
  return Str8_make(s.data + b, e - b);
}

c_linkage String8
Str8_clip_p(String8 s, void *b, void *e)
{
  U8 *bp = (U8*)b, *ep = (U8*)e;
  Assert(s.data <= bp && bp <= ep && ep <= s.data + s.size);
  return Str8_make(bp, UmmFromPtr(ep - bp));
}


////////////////////////
// Sub-String Combination:

c_linkage Umm
Str8_meld_size(String8 *sub_ptrs[], U8 sub_cnt, Umm separator_size, Umm prefix_size, Umm append_size, B8 is_nt)
{
  U8 sep_cnt = (sub_cnt > 0) ? (sub_cnt - 1) : (0);
  U8 is_nt_size = (is_nt > 0) ? 1 : (0);

  Umm subs_total_size = 0;
  while (sub_cnt > 0)
  {
    sub_cnt -= 1;
    subs_total_size += sub_ptrs[sub_cnt]->size;
  }

  return(subs_total_size + prefix_size + append_size + is_nt_size + separator_size * sep_cnt);
}

c_linkage Umm
Str8_meld_swap(String8 *sub_ptrs[], Umm old_meld_size, String8 *replacement, U8 idx)
{
  Umm result = -1;

  old_meld_size -= sub_ptrs[idx]->size;
  sub_ptrs[idx] = replacement;
  result = old_meld_size + replacement->size;

  return(result);
}

c_linkage U8
Str8_meld_find(String8 *sub_ptrs[], U8 sub_cnt, String8 *target)
{
  U8 result = -1;

  for (U8 it = 0; it < sub_cnt; it += 1)
  {
    if (sub_ptrs[it] == target)
    {
      result = it;
      break;
    }
  }

  return(result);
}

c_linkage String8
Str8_meld(void *dst, String8 *sub_ptrs[], U8 sub_cnt, String8 separator, String8 prefix, String8 append, B8 is_nt)
{
  U8 *dest = (U8*)dst;

  if (prefix.size > 0)
  {
    OSMemory_copy(dest, prefix.data, prefix.size);
    dest += prefix.size;
  }

  if (sub_cnt < 1)
  {
    OSMemory_copy(dest, sub_ptrs[0]->data, sub_ptrs[0]->size);
    dest += sub_ptrs[0]->size;
  }
  else
  {
    // Iterate sub-strings:
    U8 it = 0;
    String8 sub = {0};

    if (separator.size < 1)
    {
      // No separators:
      while(it < sub_cnt)
      {
        sub = *sub_ptrs[it];
        OSMemory_copy(dest, sub.data, sub.size);
        dest += sub.size;
        it += 1;
      }
    }
    else
    {
      // With separators:
      U8 sep_cnt = (sub_cnt > 0) ? (sub_cnt - 1) : (0);

      while(it < sep_cnt)
      {
        sub = *sub_ptrs[it];
        OSMemory_copy(dest, sub.data, sub.size);
        dest += sub.size;
        OSMemory_copy(dest, separator.data, separator.size);
        dest += separator.size;
        it += 1;
      }

      sub = *sub_ptrs[it];
      OSMemory_copy(dest, sub.data, sub.size);
      dest += sub.size;
    }
  }

  if (append.size > 0)
  {
    OSMemory_copy(dest, append.data, append.size);
    dest += append.size;
  }

  if (is_nt > 0)
  {
    *dest = '\0';
    dest += 1;
  }

  return Str8_make(dst, dest - (U8*)dst);
}


// ---------- String Search Operations ----------

c_linkage String8
Str8_find(String8 field, String8 grain, String8 *opt_remainder, String8 *opt_preceider)
{
  String8 result = {0};

  U8* field_end = field.data + field.size;
  U8* grain_end = grain.data + grain.size;
  U8* field_it  = field.data;
  U8* grain_it  = grain.data;
  U8* best_match_head = field.data;
  U8* new_match_head  = 0;
  Umm best_match_size = 0;
  Umm new_match_size  = 0;

  for (; field_it < field_end; field_it += 1)
  {
    if (*field_it == *grain_it)
    {
      //// Determine Match Length ////
      new_match_head = field_it;
      do
      {
        field_it   += 1;
        grain_it += 1;
      }
      while (*field_it == *grain_it && grain_it < grain_end);
      new_match_size = UmmFromPtr(field_it - new_match_head);

      //// Update Best Match Found ////
      if (new_match_size > best_match_size)
      {
        best_match_head = new_match_head;
        best_match_size = new_match_size;

        //// Early-out on Precise Match or if longer match impossible ////
        if (new_match_size == grain.size || new_match_size > UmmFromPtr(field_end - field_it))
        {
          break;
        }
      }

      grain_it = grain.data;
    } // ! *field == *grain

  } // for (field_it...)

  //// Fill Result ////
  result = Str8_make(best_match_head, best_match_size);
  if (opt_remainder != 0)
  {
    *opt_remainder = Str8_make(field_it, UmmFromPtr(field_end - field_it));
  }
  if (opt_preceider != 0)
  {
    *opt_preceider = Str8_make(field.data, UmmFromPtr(best_match_head - field.data));
  }

  return (result);
}


c_linkage String8
Str8_find_r(String8 field, String8 grain, String8 *opt_remainder, String8 *opt_preceider)
{
  String8 result = {0};

  U8* field_start = field.data;
  U8* grain_start = grain.data;
  U8* field_it    = field.data + field.size - 1;
  U8* grain_it    = grain.data + grain.size - 1;
  U8* best_match_tail = field.data + field.size;
  U8* new_match_tail  = 0;
  Umm best_match_size = 0;
  Umm new_match_size  = 0;

  for (; field_it >= field_start; field_it -= 1)
  {
    if (*field_it == *grain_it)
    {
      //// Determine Match Length ////
      new_match_tail = field_it + 1;
      do
      {
        field_it   -= 1;
        grain_it -= 1;
      }
      while (*field_it == *grain_it && grain_it >= grain_start);
      new_match_size = UmmFromPtr(new_match_tail - field_it - 1);

      //// Update Best Match Found ////
      if (new_match_size > best_match_size)
      {
        best_match_tail = new_match_tail;
        best_match_size = new_match_size;

        //// Early-out on Precise Match or if longer match impossible ////
        if (new_match_size == grain.size || new_match_size > UmmFromPtr(field_it - field_start))
        {
          break;
        }
      }

      grain_it = grain.data + grain.size - 1;
    } // ! *field == *grain

  } // for (field_it...)

  //// Fill Result ////
  result = Str8_make(best_match_tail - best_match_size, best_match_size);
  if (opt_remainder != 0)
  {
    *opt_remainder = Str8_make(field_start, UmmFromPtr(field_it + 1 - field_start));
  }
  if (opt_preceider != 0)
  {
    *opt_preceider = Str8_make(best_match_tail, field.size - UmmFromPtr(best_match_tail - field_start));
  }

  return (result);
}


