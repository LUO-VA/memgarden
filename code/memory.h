#ifndef MEMORY_H
#define MEMORY_H

// #include "habit.h"
// #include "os.h"


/******************************************************************************
 * MEMORY.H
 *
 * DESCRIPTION:
 * Basic memoryblock allocation scheme.
 *
******************************************************************************/

////////////////////////
// MemoryArena Structure:

typedef struct MemoryArenaBlock MemoryArenaBlock;
struct MemoryArenaBlock
{
  MemoryArenaBlock *prev;
  Umm cap; // reserved.
  Umm bar; // committed.
  Umm hop; // used.
};

typedef U8 MemoryArenaFlags;
enum MEMORY_ARENA_FLAGS {
  MARENA_LIST = 1, // Allow allocation of new memoryblocks.
  MARENA_GLUE = 2, // Allocate new memoryblock to end of previous one.
  MARENA_KEEP = 4, // Prevent releasing memoryblocks to OS.
};

typedef struct {
  // Allocation strategy:
  Umm cap_min; // Minimum reserve size.
  Umm bar_min; // Minimum de-/commit size.
  Umm cap_align; // allocation_granularity
  Umm bar_align; // page_size
  // TODO : dirty_region (popped area that's still committed should be zeroed when asked).

  // Blockchain:
  MemoryArenaBlock *current;

  OSContext *os_ctx;

  // Behaviour:
  MemoryArenaFlags flags;
} MemoryArena;


////////////////////////
// MemoryArena Private Functions:

internal MemoryArenaBlock* MemoryArenaBlock_make(MemoryArenaBlock *prev, U8 *opt_addr, Umm cap, Umm bar, Umm cap_align, Umm bar_align);

// These return 0 on failure.
internal B8 MemoryArena_block_push(MemoryArena *arena, Umm size, Umm alignment);
internal B8 MemoryArena_block_pop(MemoryArena *arena);
internal B8 MemoryArenaBlock_kill(MemoryArenaBlock *block);


////////////////////////
// MemoryArena Public Functions:

c_linkage MemoryArena MemoryArena_make(Umm init_cap, Umm init_bar, Umm cap_min, Umm bar_min, Umm cap_align, Umm bar_align, U8 *opt_addr, MemoryArenaFlags flags);

// Kill to free all including base block (doesn't care if MARENA_KEEP set).
// Returns 0 on failure.
c_linkage B8 MemoryArena_kill(MemoryArena *arena);

// Returns properly aligned address to allocated memory, 0 on failure.
c_linkage U8* MemoryArena_push_size(MemoryArena *arena, Umm size, Umm alignment);

// Returns address to beginning of freed memory, 0 on failure.
// Given address must be inside arenas current block.
c_linkage U8* MemoryArena_pop_to(MemoryArena *arena, U8 *addr);


#endif // !MEMORY_H
