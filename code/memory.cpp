//#include "memory.h"

// ==================== Private Functions ====================

internal MemoryArenaBlock*
MemoryArenaBlock_make(MemoryArenaBlock *prev, U8 *opt_addr, Umm cap, Umm bar, Umm cap_align, Umm bar_align)
{
  // TODO : Assert(...);
  MemoryArenaBlock *result = 0;

  //////////////////// RESERVE:
  cap = Align(cap, cap_align);

  U8 *reserve_memory = OSMemoryBlock_request(opt_addr, cap, cap_align, 0, OSMEM_RESERVED);

  if (reserve_memory != 0)
  {
    //////////////////// COMMIT:
    bar = Align(bar, bar_align);
    U8 *commit_memory = OSMemoryBlock_request(reserve_memory, bar, bar_align, 0, OSMEM_COMMITTED);

    if(commit_memory == reserve_memory)
    {
      result = (MemoryArenaBlock*)reserve_memory;
      result->prev = prev;
      result->cap = cap;
      result->bar = bar;
      result->hop = sizeof(MemoryArenaBlock);
    }
    else
    {
      // On failure to commit release reserve.
      U8 *release_memory = OSMemoryBlock_request(reserve_memory, cap, cap_align, 0, OSMEM_RELEASED);
      if (release_memory == 0)
      {
        // TODO : @LOG
      }
    }
  }

  return(result);
}


internal B8
MemoryArenaBlock_kill(MemoryArenaBlock *block)
{
  B8 result = 0;

  U8 *release_memory = OSMemoryBlock_request((U8*)block, block->cap, 0, 0, OSMEM_RELEASED);

  if (release_memory != 0)
  {
    result = 1;
  }
  else
  {
      // TODO : @LOG
  }

  return(result);
}


internal B8
MemoryArena_block_push(MemoryArena *arena, Umm size, Umm alignment)
{
  // TODO : Assert(...);
  B8 result = 0;

  MemoryArenaBlock *current_block = arena->current;

  if (arena->flags & MARENA_LIST != 0)
  {
    // Account for header at beginning of reserve block.
    if (alignment != 0) {
      size += Align(sizeof(MemoryArenaBlock), alignment);
    }
    else {
      size += sizeof(MemoryArenaBlock);
    }

    Umm cap = (size >= arena->cap_min) ? size : arena->cap_min;
    Umm bar = (size >= arena->bar_min) ? size : arena->bar_min;
    U8 *addr = 0;
    if (arena->flags & MARENA_GLUE)
    {
      addr = PtrFromUmm(UmmFromPtr(current_block) + current_block->cap);
    }

    MemoryArenaBlock *new_block = MemoryArenaBlock_make(current_block->prev, addr, cap, bar, arena->cap_align, arena->bar_align);

    if (new_block != 0)
    {
      arena->current = new_block;
      result = 1;
    }
  }

  return(result);
}


internal B8
MemoryArena_block_pop(MemoryArena *arena)
{
  // TODO : Assert(...);
  B8 result = 0;

  MemoryArenaBlock *current_block = arena->current;

  if (current_block->prev != 0 && arena->flags & MARENA_KEEP != 0)
  {
    MemoryArenaBlock *prev_block = current_block->prev;
    if (MemoryArenaBlock_kill(current_block) != 0)
    {
      arena->current = prev_block;
      result = 1;
    }
  }

  return(result);
}



// ==================== Public Functions ====================

c_linkage MemoryArena
MemoryArena_make(Umm init_cap, Umm init_bar, Umm cap_min, Umm bar_min, Umm cap_align, Umm bar_align, U8 *opt_addr, MemoryArenaFlags flags)
{
  // TODO : Assert(cap/bar_align > 0, ...);
  MemoryArena result = {0};

  init_cap = Align(init_cap, cap_align);
  init_bar = Align(init_bar, bar_align);

  // RESERVE
  MemoryArenaBlock *base_block = 
    MemoryArenaBlock_make(0, opt_addr, init_cap, init_bar, cap_align, bar_align);

  if (base_block != 0)
  {
    // COMMIT
    result.current = base_block;
    result.cap_min = cap_min;
    result.bar_min = bar_min;
    result.cap_align = cap_align;
    result.bar_align = bar_align;
    result.flags = flags;
  }

  return(result);
}


c_linkage B8
MemoryArena_kill(MemoryArena *arena)
{
  // TODO : Assert(...);
  B8 result = 0;

  MemoryArenaBlock *current = arena->current;
  MemoryArenaBlock *prev;

  while(arena->current != 0)
  {
    prev = current->prev;
    if (MemoryArenaBlock_kill(current) == 0)
    {
      break;
    }
    current = prev;
  };

  if (arena->current == 0)
  {
    result = 1;
  }

  return(result);
}


c_linkage U8*
MemoryArena_push_size(MemoryArena *arena, Umm size, Umm alignment)
{
  // TODO : Assert(..., alignment != 0);
  U8 *result = 0;

  MemoryArenaBlock dummy_block = {0};

  // Setup requested memory region:
  MemoryArenaBlock *block = (arena->current != 0) ? arena->current : &dummy_block;

  Umm block_pos = UmmFromPtr(arena->current);
  Umm aligned_pos = EffectiveAddress(block_pos, block->hop, alignment);
  Umm cap_bound = block_pos + block->cap;
  Umm bar_bound = block_pos + block->bar;
  Umm new_hop = aligned_pos + size - block_pos;

  if (new_hop > cap_bound)
  {
    // =============== NEW-BLOCK: ================
    if (MemoryArena_block_push(arena, size, alignment) != 0)
    {
      block = arena->current;
      block_pos = UmmFromPtr(block);
      aligned_pos = EffectiveAddress(block, block->hop, alignment);
      new_hop = aligned_pos + size - block_pos;
    }
    else
    {
      block = &dummy_block;
    }
  }
  else 
  {
    if (new_hop > bar_bound)
    {
      // =============== RAISE-BAR: ================
      U8 *addr = PtrFromUmm(block_pos + bar_bound);
      U8 *commit_memory = OSMemoryBlock_request(addr, size, arena->bar_align, 0, OSMEM_COMMITTED);
      if (commit_memory == 0)
      {
        block = &dummy_block;
      }
    }
  }

  if (block != &dummy_block)
  {
    // =============== RAISE-HOP: ================
    block->hop = new_hop;
    result = PtrFromUmm(aligned_pos);
  }

  return(result);
}

c_linkage U8*
MemoryArena_pop_to(MemoryArena *arena, U8 *addr)
{
  // TODO : Assert(...);
  U8 *result = 0;

  MemoryArenaBlock *block = arena->current;

  Umm block_pos = UmmFromPtr(block);
  Umm cap_bound = block_pos + block->cap;
  Umm new_pos = UmmFromPtr(addr);

  if (new_pos >= block_pos && new_pos < cap_bound)
  {
    Umm new_hop = new_pos - block_pos;

    if (new_hop == 0)
    {
      // =============== REMOVE-BLOCK: ================
      if (MemoryArena_block_pop(arena) != 0)
      {
        // Result = previous blocks hop.
        block_pos = UmmFromPtr(arena->current);
        result = PtrFromUmm(block_pos + block->hop);
      }
    }
    else
    {
      Umm bar_bound = block_pos + block->bar;
      if (new_hop < bar_bound)
      {
        // =============== LOWER-BAR: ================
        Umm bar_fall = bar_bound - new_pos;
        if (bar_fall >= Align(arena->bar_min, arena->bar_align))
        {

        }

        if (new_hop < block->hop)
        {
          // =============== LOWER-HOP: ================
          block->hop = new_hop;
        }
      }
      result = PtrFromUmm(block_pos + block->hop);
    }
  }

  return(result);
}

