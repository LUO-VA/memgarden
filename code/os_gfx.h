#ifndef OS_GFX_H
#define OS_GFX_H

//#include "os.h"


/******************************************************************************
 *
 * OS/GFX.H
 *
 * DESCRIPTION:
 * Interface for window creation and graphics backend initialization.
 * Provides a communication method between os and a renderer.
 *
 * USAGE:
 * os_.h files provide an abstraction interface.
 * >> typedef Foo_Init(Bar bar); // Provided function signature.
 * >> static  Foo_init(Bar bar); // Declaration in [impl].h.
 * >> Foo_init(bar);             // User code.
 *
 * TODO :
 * MonitorRefreshRate etc...
 *
******************************************************************************/



// ==================== Graphics Sentinel ====================

//#define OS_GRAPHICS_SENTINEL_ALLOC_SIZE // Impl -> 0 if user doesn't provide memory.
typedef void OSGraphicsSentinel;
typedef struct OSGraphicsCallback OSGraphicsCallback; // User filled.
typedef OSGraphicsSentinel * OSGraphicsSentinel_Init(OSGraphicsSentinel *gfx_sentinel_mem, OSGraphicsCallback *os_gfx_user_callback, OSContext *os_ctx);



// ==================== Backend Management ====================
// Replace OS_GRAPHICS... with [backend_name]..., e.g. Win32OpenGL...

//#define OS_GRAPHICS_BACKEND_ALLOC_SIZE // Impl -> 0 if user doesn't provide memory.
typedef void OSGraphicsBackend;
typedef OSGraphicsBackend * OSGraphicsBackend_Init(OSGraphicsBackend *gfx_backend_mem, OSGraphicsSentinel *gfx_sentinel);


// ==================== Window Handling ====================

// ---------- setup ----------

//#define OS_WINDOW_COUNT_DESIRED  // User -> Provide before include.
#ifdef OS_WINDOW_COUNT_DESIRED
# if OS_WINDOW_COUNT_DESIRED < 1
#  undef OS_WINDOW_COUNT_DESIRED
# endif
#endif
#ifndef OS_WINDOW_COUNT_DESIRED
# define OS_WINDOW_COUNT_DESIRED 1
#endif
//#define OS_WINDOW_COUNT // Impl -> Actual initial window count.


// ---------- actions ----------

typedef struct {
  S32 x, y, w, h;
  // TODO : title
  // TODO : Options (hidden,...)
} OSWindowSpec;

typedef void OSWindow;

// These return 0 on failure (_kill considers invalid id's as success):
typedef OSWindow * OSWindow_Make(OSWindowSpec *spec, void *user_window_data, OSGraphicsBackend *gfx_backend);
typedef B8 OSWindow_Kill(OSWindow *os_window);



// ==================== User Callback ====================
// Called at end of successfull operation, e.g. after window is destroyed.

// Callback signatures:
typedef void OSWindow_Kill_Callback(OSWindow *os_window, void *user_callback_data, void *user_window_data);
// [@Unsafe - os_window may nolonger exist, ptr provided for book-keeping.]

struct OSGraphicsCallback {
  OSWindow_Kill_Callback *window_kill;
  void *data;
};



// ==================== Render Facility ====================
// Provided by each backends impl.h file.

// Render/Refresh display buffer, returns 0 on failure.
typedef B8 OSWindow_Display_Update(OSWindow *os_window);


#endif // !OS_GFX_H
