#ifndef OS_WIN32_H
#define OS_WIN32_H

//#include "os.h"


/******************************************************************************
 * OS_WIN32.H
 *
 * DESCRIPTION:
 *
 * ===========================================================================
******************************************************************************/


// ==================== Prerequisites ====================

#include <Windows.h>
//#pragma comment(lib, "kernel32")



// ==================== OS Context ====================

#define OS_CONTEXT_ALLOC_SIZE 0
c_linkage OSContext * OS_init(OSContext *os_ctx_mem);


// ---------- run tree ----------

#define OS_DIRECTORY_SEPARATOR "\\"
c_linkage U16 OSContext_get_mainproc_fullpath(void **path_ptr_receiver, OSContext *os_ctx);



// ==================== Program Loop ====================
//
// Windows crt entrypoint if using main();
// >> extern "C" int __stdcall WinMainCRTStartup()
//

#define OS_PROGRAM_ENTRYPOINT \
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, \
                     PSTR lpCmdLine, int nCmdShow) {

#define OS_PROGRAM_EXITPOINT(exitcode) return((LRESULT)exitcode); }


c_linkage B8 OS_main_tick(OSContext *os_ctx);
// Calls Win32TickHooks.


// ---------- program loop hooks ----------

typedef struct Win32MainTickHook Win32MainTickHook;
// Hook function return type is next hook to call.

typedef Win32MainTickHook * Win32MainTick_Call(Win32MainTickHook *hook);
// Hook function signature (takes itself as argument).

internal Win32MainTickHook * Win32MainTick_stub(Win32MainTickHook *hook);
// Initial hook returning itself.

c_linkage void Win32MainTickHook_strap(Win32MainTickHook *hook);
// O(1) insertion. [@Unsafe - _strap doens't check if dispatch already exists.]

c_linkage void Win32MainTickHook_sever(Win32MainTickHook *hook);
// O(N) removes. [@Unsafe - _sever doens't check for infinite loop.]

struct Win32MainTickHook {
  Win32MainTickHook *next;
  Win32MainTick_Call *call;
  void *data;
};



// ==================== os - Memory Management ====================

// ---------- common tools ----------

c_linkage void OSMemory_zero(void *data, Umm size);
c_linkage void OSMemory_copy(void *dst, void *src, Umm size);


// ---------- memory functions ----------

c_linkage OSMemoryScheme OSMemoryScheme_inquire(void);
c_linkage U8 * OSMemoryBlock_request(U8 *base, Umm size, Umm alignment,
                                   OSMemoryPageProperties properties,
                                   OSMemoryPageState state);



// ==================== os - File I/O ====================

#define OSFILE_INVALID INVALID_HANDLE_VALUE

c_linkage OSFile * OSFile_make(void *fullpath, OSFileModeFlags flags);
c_linkage B8       OSFile_cull(void *fullpath);
c_linkage B8       OSFile_kill(OSFile * os_file);
c_linkage U64      OSFile_size(OSFile *os_file, U64 size);
c_linkage Umm      OSFile_read(OSFile *os_file, void *dst, Umm offs, Umm size);
c_linkage Umm      OSFile_write(OSFile *os_file, void *src, Umm offs, Umm size);

#endif // !OS_WIN32_H
