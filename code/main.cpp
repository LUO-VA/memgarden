#define ENABLE_ASSERT
#include "habit.h"

#define _AVX2
#define _AVX
#define _FMA3
#define _SSE42
#define _SSE41
#define _SSSE3
#define _SSE3
#define _SSE2
//#define _SSE
#include "vecmat_x86.h"
#include "vecmat_emu.h"

#define OS_WINDOW_COUNT_DESIRED 3
#include "os.h"
#include "os_gfx.h"
#include "os_win32.h"
#include "os_win32_gfx.h"
#include "os_win32_gfx_software.h"
#include "os_win32.cpp"
#include "os_win32_gfx.cpp"
#include "os_win32_gfx_software.cpp"

#include "memory.h"
#include "memory.cpp"

#include "string.h"
#include "string.cpp"

#include "bmp.h"

#include "sore.h"


typedef struct
{
  //// OS ////
  OSContext *os_ctx;
  OSMemoryScheme os_memory_scheme;

  // os_run-tree:
  String8 os_dir_separator;
  String8 mainproc_fullpath;

  //// OS/GFX ////
  OSGraphicsCallback *gfx_callback;
  OSGraphicsSentinel *gfx_sentinel;
  OSGraphicsBackend *gfx_software_backend;
  OSWindow *gfx_windows[OS_WINDOW_COUNT];

  //// RUN-TREE ////
  String8 program_dir;
  String8 program_filename;

  // directories:
  String8 runtree_dir;

  // folders:
  #define FOLDER_ASSETS "assets"OS_DIRECTORY_SEPARATOR
  String8 assets_folder;

  // files:
  #define FILE_FG "fg.bmp"

  // paths:
  String8 fg_path;

  //// MEMORY ////
  MemoryArena *main_arena;

  // assets:
  PixelBuffer fg;

} ProgramStructure;


internal void OSWindow_kill_callback(OSWindow *os_window, void *user_callback_data, void *user_window_data)
{
  if (user_window_data != 0)
  {
    OSWindow** program_gfx_window = (OSWindow**)user_window_data;
    *program_gfx_window = 0;
  }
}


OS_PROGRAM_ENTRYPOINT


// ==================== SETUP PROGRAM MAIN MEMORY ====================

// ---------- Initial Allocation ----------

OSMemoryScheme os_memory_scheme = OSMemoryScheme_inquire();
Umm allocation_granularity = os_memory_scheme.alignments.allocation_granularity;
Umm default_page_size = os_memory_scheme.alignments.page_small;

MemoryArena main_arena = MemoryArena_make(GiB(1), MiB(128), MiB(2), KiB(256), allocation_granularity, default_page_size, 0, 0);

// Bootstrap main arena.
MemoryArena *main_arena_bootstrap = (MemoryArena*) MemoryArena_push_size(&main_arena, sizeof(MemoryArena), 1);
*main_arena_bootstrap = main_arena;


// ---------- BootStrap Program Structure ----------

ProgramStructure *program = (ProgramStructure*) MemoryArena_push_size(&main_arena, sizeof(ProgramStructure), 1);
program->main_arena       = main_arena_bootstrap;
program->os_memory_scheme = os_memory_scheme;

program->gfx_callback = (OSGraphicsCallback*) MemoryArena_push_size(program->main_arena, sizeof(OSGraphicsCallback), 1);
program->gfx_callback->window_kill = OSWindow_kill_callback;
program->gfx_callback->data = 0;



// ==================== SETUP PROGRAM OS LAYERS ====================

Umm os_layer_total_alloc_size = OS_CONTEXT_ALLOC_SIZE + OS_GRAPHICS_SENTINEL_ALLOC_SIZE + OS_SOFTWARE_GRAPHICS_BACKEND_ALLOC_SIZE;

U8* os_layer_memory = MemoryArena_push_size(program->main_arena, os_layer_total_alloc_size, 1);


// ---------- Init Context ----------

program->os_ctx = OS_init(os_layer_memory);
os_layer_memory += OS_CONTEXT_ALLOC_SIZE;


// ---------- Init Graphics Sentinel ----------

program->gfx_sentinel = OSGraphicsSentinel_init(os_layer_memory, program->gfx_callback, program->os_ctx);
os_layer_memory += OS_GRAPHICS_SENTINEL_ALLOC_SIZE;


// ---------- Init Graphics Backends ----------

program->gfx_software_backend = Swg_init(os_layer_memory, program->gfx_sentinel);
os_layer_memory += OS_SOFTWARE_GRAPHICS_BACKEND_ALLOC_SIZE;



// ==================== SETUP PROGRAM RUN_TREE ====================

// ---------- Executable Filepath ----------

program->os_dir_separator = Str8_lit(OS_DIRECTORY_SEPARATOR);

program->mainproc_fullpath.size = OSContext_get_mainproc_fullpath((void**)&program->mainproc_fullpath.data, program->os_ctx);


// ---------- Setup Run-Tree ----------

// Find last slash:
Str8_find_r(program->mainproc_fullpath, program->os_dir_separator, &program->program_dir, &program->program_filename);

// Put last slash into dir path.
program->program_dir.size += program->os_dir_separator.size;

program->runtree_dir = program->program_dir;


////////////////////////
// Folders:

program->assets_folder = Str8_lit(FOLDER_ASSETS);


////////////////////////
// Files:

U8 *filepath_mem;
String8 target_file;
String8 str8_zero = Str8_zero();
String8 *filepath_melding[3] = { &program->runtree_dir, &program->assets_folder, &str8_zero};
Umm filepath_meldsize = Str8_meld_size(filepath_melding, 3, 0, 0, 0, 0);

// Foreground image:
target_file       = Str8_lit(FILE_FG);
filepath_meldsize = Str8_meld_swap(filepath_melding, filepath_meldsize, &target_file, 2);
filepath_mem      = MemoryArena_push_size(program->main_arena, filepath_meldsize, 1);
program->fg_path  = Str8_meld(filepath_mem, filepath_melding, 3, str8_zero, str8_zero, str8_zero, 1);


// ==================== LOAD ASSETS ====================

// Foreground image:
SORE_load_bitmap(program->main_arena, &program->fg, &program->fg_path);




// ==================== CREATE MAIN WINDOW ====================

OSWindowSpec os_window_spec = {0};
os_window_spec.w = 960;
os_window_spec.h = 540;

program->gfx_windows[0] = OSWindow_make(&os_window_spec, &program->gfx_windows[0], program->gfx_software_backend);



// ---------- Setup Graphics Backend ----------

Swg_set_display_buffer(program->gfx_windows[0], program->fg.data, program->fg.w, program->fg.h);



// ==================== PROGRAM MAIN LOOP ====================

bool app_running = 1;
while (app_running = OS_main_tick(program->os_ctx) && program->gfx_windows[0] != 0)
{
  OSWindow_display_update(program->gfx_windows[0]);

  //////////////////// Preliminary SIMD Tests:

  float floats_a[4] = {1,2,3,4};
  float floats_b[4] = {5,6,7,8};

  V4F32 a = V4F32_load(floats_a);
  V4F32 b = V4F32_load(floats_b);
  V4F32 c = V4F32_add(a,b);
  V4F32 d = V4F32_sub(b,a);
  V4F32 e = V4F32_mul(a,b);
  V4F32 f = V4F32_div(b,a);

  V4F32 q = V4F32_mul(c, V4F32_div(V4F32_add(e, b), V4F32_sub(d, f)));
}


OS_PROGRAM_EXITPOINT(0)


