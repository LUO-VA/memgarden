#ifndef OS_WIN32_GFX_SOFTWARE_H
#define OS_WIN32_GFX_SOFTWARE_H

//#include "os_win32_gfx.h"

/******************************************************************************
 * OS_WIN32_GFX_SOFTWARE.H [SOFTWARE GRAPHICS BACKEND]
 *
 * DESCRIPTION:
 *
******************************************************************************/


// ==================== os_win32/gfx - Backend Management ====================

#define OS_SOFTWARE_GRAPHICS_BACKEND_ALLOC_SIZE 0
c_linkage OSGraphicsBackend * Swg_init(OSGraphicsBackend *swg_backend_mem, OSGraphicsSentinel *os_gfx_sentinel);


// ==================== win32/gfx - Window Handling ====================

// GfxBackend type.
typedef struct SwgWindow SwgWindow;

c_linkage GfxWindowBackend * Swg_window_make(OSWindowSpec *spec);
c_linkage B8                 Swg_window_kill(GfxWindowBackend *gfx_window_backend);


// ***** SoftwareGraphics *****

struct SwgWindow {
  // Other windows associated data:
  SwgWindow *next; // 0 unless in free list.

  // Render provided display buffer.
  void *rgba32_buffer;

  // Ms internal display buffer:
  BITMAPINFO bmi;
};



// ==================== win32/gfx Render Facility ====================

c_linkage B8 Swg_display_update(OSWindow *os_window);


// ***** SoftwareGraphics Render Api *****

c_linkage B8 Swg_set_display_buffer(OSWindow *os_window, void *rgba32_buffer, S32 w, S32 h);
// Detach by giving 0 pointer to buffer.



#endif // !OS_WIN32_GFX_SOFTWARE_H
