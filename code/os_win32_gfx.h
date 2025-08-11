#ifndef OS_WIN32_GFX_H
#define OS_WIN32_GFX_H

//#include "os_win32.h"
//#include "os_gfx.h"


/******************************************************************************
 * OS/GFX_WIN32.H [GRAPHICS MODULE] --------------------
 *
 * DESCRIPTION:
 *
******************************************************************************/

// ==================== Prerequisites ====================

#pragma comment(lib, "User32") // required for windows.
#pragma comment(lib, "Gdi32") // required for graphics (e.g. bit-block-transfer ops).



// ==================== Graphics Sentinel ====================

#define OS_GRAPHICS_SENTINEL_ALLOC_SIZE 0
c_linkage OSGraphicsSentinel * OSGraphicsSentinel_init(OSGraphicsSentinel *gfx_sentinel_mem, OSGraphicsCallback *os_gfx_user_callback, OSContext *os_ctx);


// ***** Win32 *****

internal Win32MainTickHook * GfxSentinel_main_tick_call(Win32MainTickHook *hook);
// MS message handling for all windows.



// ==================== Backend Management ====================

// OSGraphicsBackend type.
typedef struct GfxBackend GfxBackend;


// ***** Win32GraphicsBackend Dispatch Table *****

// Window management function signatures:
typedef void GfxWindowBackend; // Backend window instance data.
typedef GfxWindowBackend * GfxBackend_Window_Make(OSWindowSpec *spec);
typedef B8 GfxBackend_Window_Kill(GfxWindowBackend *gfx_window_backend);

// Render facility function signatures:
typedef B8 GfxBackend_Window_Display_Update(OSWindow *os_window);

// Backend dispatch table.
struct GfxBackend {
  void *data;
  GfxBackend_Window_Make *window_make;
  GfxBackend_Window_Kill *window_kill;
  GfxBackend_Window_Display_Update *display_update;
};



// ==================== Window Handling ====================

// OSWindow type.
typedef struct GfxWindow GfxWindow;

#define OS_WINDOW_COUNT OS_WINDOW_COUNT_DESIRED
#define OS_WINDOW_LIST_SUPPORT 0

c_linkage OSWindow * OSWindow_make(OSWindowSpec *spec, void *user_window_data, OSGraphicsBackend *os_gfx_backend);
c_linkage B8         OSWindow_kill(OSWindow *os_window);


// ***** Win32 *****

struct GfxWindow {
  GfxWindow *next;
  GfxWindow *prev;

  // Backend (dispatch / window instance data):
  GfxBackend *backend_dispatch;
  GfxWindowBackend *backend_data;

  // User
  void *user_data;


  // MS:
  HWND ms_window_handle;

  // os_gfx:
  OSWindowSpec spec;

  // Validity check (window exists and has a backend):
  B8 is_valid;
};


// ***** MS *****

internal LRESULT CALLBACK MSWindowProcedure_default(HWND ms_window_handle, UINT message, WPARAM param_hi, LPARAM param_lo);
// MS WndProc callback required for windows message handling.

internal GfxWindow * GfxWindow_from_hwnd(HWND ms_window_handle);
// MS WndProc access to windows.



// ==================== User Callback ====================
// Stubs

internal void OSWindow_kill_callback_stub(OSWindow *os_window, void *user_callback_data, void *user_window_data);



// ==================== Render Facility ====================
// Provided by each backends impl.h file.

c_linkage B8 OSWindow_display_update(OSWindow *os_window);



#endif // !OS_WIN32_GFX_H
