//#include "os_win32_gfx_software.h"


// ==================== os_win32/gfx - Backend Management ====================

GfxBackend gfx_swg_backend = {0};

SwgWindow swg_window_roster[OS_WINDOW_COUNT] = {0};
SwgWindow *swg_window_free = 0;


c_linkage OSGraphicsBackend *
Swg_init(OSGraphicsBackend *swg_backend_mem, OSGraphicsSentinel *os_gfx_sentinel)
{
  // TODO : Assert(...);

  GfxBackend *result = 0;

  // Init bitmap lists:
  swg_window_free = swg_window_roster;
  SwgWindow *last = swg_window_roster + ArrayCount(swg_window_roster) - 1;
  SwgWindow *iter = swg_window_roster;
  for (; iter < last; iter += 1)
  {
    iter->next = iter + 1;
  }
  iter->next = 0;

  // Init Win32GraphicsBackend (dispatch):
  gfx_swg_backend.window_make = Swg_window_make;
  gfx_swg_backend.window_kill = Swg_window_kill;
  gfx_swg_backend.display_update = Swg_display_update;
  //win32_swg_backend.data = swg_window_roster;

  result = &gfx_swg_backend;

  return(result);
}



// ==================== win32/gfx - Window Management ====================

c_linkage GfxWindowBackend *
Swg_window_make(OSWindowSpec *spec)
{
  // TODO : Assert(spec.w/h > 0, ...)

  GfxWindowBackend *result = 0;

  if (swg_window_free != 0)
  {
    SwgWindow *swg_window = swg_window_free;

    // Update window backend data listing:
    swg_window_free = swg_window_free->next;
    swg_window->next = 0;

    result = (GfxBackend*)swg_window;
  }



  return(result);
}


c_linkage B8
Swg_window_kill(GfxWindowBackend *gfx_window_backend)
{
  // TODO : Assert(...);

  B8 result = 1;

  SwgWindow *swg_window = (SwgWindow*)gfx_window_backend;

  // Update window backend data listing:
  swg_window->next = swg_window_free;
  swg_window_free = swg_window;

  return(result);
}



// ==================== Render Facility ====================

c_linkage B8
Swg_display_update(OSWindow *os_window)
{
  // TODO : Assert(...);

  B8 result = 0;

  GfxWindow *gfx_window = (GfxWindow*)os_window;
  SwgWindow *swg_window = (SwgWindow*)gfx_window->backend_data;

  HWND ms_window_handle = gfx_window->ms_window_handle;

  if (ms_window_handle != 0)
  {
    HDC device_context_handle = GetDC(ms_window_handle); // 0 = screen (draw to monitor).

    if (device_context_handle != NULL)
    {
      // Bit-block transfer:
      int oriented_scanline_count = StretchDIBits(
        device_context_handle,

        // Dst(x,y,w,h), Src(x,y,w,g):
        0, 0, gfx_window->spec.w, gfx_window->spec.h,
        0, 0, swg_window->bmi.bmiHeader.biWidth, swg_window->bmi.bmiHeader.biHeight,

        swg_window->rgba32_buffer, // ImageBits.
        &swg_window->bmi,          // MS bitmapinfo structure.
        0,       // Usage - 0 unless using color table or compression.
        SRCCOPY  // RasterOp (must be SRCCOPY if compression involved).
      );

      if (oriented_scanline_count > 0)
      {
        result = 1;
      }

      if (ReleaseDC(ms_window_handle, device_context_handle) != 1)
      {
        // TODO : @LOG
      }
    } // !device_context_handle
  } // !ms_window_handle

  return(result);
}


c_linkage B8
Swg_set_display_buffer(OSWindow *os_window, void *rgba32_buffer, S32 w, S32 h)
{
  // TODO : Assert(w & h > 0, ...);

  B8 result = 1;

  GfxWindow *gfx_window = (GfxWindow*)os_window;
  SwgWindow *swg_window = (SwgWindow*)gfx_window->backend_data;

  if (rgba32_buffer == 0)
  {
    swg_window->rgba32_buffer = 0; // Detach display buffer.
  }
  else
  {
    swg_window->rgba32_buffer = rgba32_buffer;
    swg_window->bmi = BICA(BITMAPINFO) {
      //// BITMAPINFOHEADER bmiHeader: ////
      sizeof(BITMAPINFOHEADER),
      w, h,   // width/height
      1,      // color_planes
      32,     // bits_per_pixel
      BI_RGB, // Compression - BI_RGB = uncompressed (not JPEG/PNG).
      0,      // Size (can be 0 for uncompressed RGB bitmaps).
      1, 1,   // x/yPelsPerMeter - for printers?
      0,      // DesiredColorCount - Number of indecies used from table.
      0,      // RequiredColorCount - Important indecies for displaying bitmap (0 = all).

      //// RGBQUAD bmiColors[1]: ////
      // If using color table, this is it's first member (array continues after this struct).
      0
    };
  }


  return(result);
}

