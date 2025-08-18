//#include "os_win32_gfx.h"

/******************************************************************************
 *
 * GENERAL GRAPHICS BACKEND SUITABLE WINDOW
 *
 * SOFTWARE REQUIREMENTS:
 * None.
 *
 * OPENGL REQUIREMENTS:
 * OpenGL window class attribute should not include CS_PARENTDC and
 * the window should be created with the WS_CLIPCHILDREN | WS_CLIPSIBLINGS styles set.
 * This is due to an OpenGL window having its own pixel format and thus only
 * device contexts retrieved for the client area are allowed to draw into the window.
 *
******************************************************************************/


// ==================== GLOBALS ====================

// Generic window type:
global ATOM ms_general_window_class_atom;

// User Callback (initial dummy provided):
global OSGraphicsCallback *gfx_user_callback = 0;
global OSGraphicsCallback gfx_user_callback_dummy = {0};
internal void OSWindow_kill_callback_stub(OSWindow *os_window, void *user_callback_data, void *user_window_data) { }

// OSGraphicsSentinel:
global GfxWindow gfx_window_roster[OS_WINDOW_COUNT] = {0};
global GfxWindow *gfx_window_head = 0;
global GfxWindow *gfx_window_free = 0;

// OSMainTick entry:
// MS window message handler.
global Win32MainTickHook gfx_sentinel_main_tick_hook = {0};



// ==================== win32/gfx Graphics Sentinel ====================

c_linkage OSGraphicsSentinel *
OSGraphicsSentinel_init(OSGraphicsSentinel *gfx_sentinel_mem, OSGraphicsCallback *os_gfx_user_callback, OSContext *os_ctx)
{
  // TODO : Assert(...)

  OSGraphicsSentinel *result = 0;

  // Register window class suitable for graphics backends.
  WNDCLASSEXA window_class_description = {
    sizeof(WNDCLASSEXA),          // cbSize
    CS_OWNDC,                     // UINT style
    MSWindowProcedure_default,    // WNDPROC
    0, 0,                         // Extra bytes at end of class struct.
    win32_main_module_handle,     // HINSTANCE
    0, 0, 0, 0,                   // icon, cursor, background brush, menu name.
    "win32_general_window_class", // lpszClassName
    0                             // small icon.
  };
  ms_general_window_class_atom = RegisterClassExA(&window_class_description);

  if (ms_general_window_class_atom != 0)
  {
    // Init window lists:
    gfx_window_free = gfx_window_roster;
    gfx_window_head = 0;

    GfxWindow *last = gfx_window_roster + ArrayCount(gfx_window_roster) - 1;
    GfxWindow *iter = gfx_window_roster;
    iter->prev = 0, last->next = 0;
    for (GfxWindow *iter = gfx_window_roster; iter < last; iter += 1)
    {
      iter->next = iter + 1;
      iter->next->prev = iter;
    }
    last->prev = iter;


    // Setup main loop hooks:
    gfx_sentinel_main_tick_hook.call = GfxSentinel_main_tick_call;
    gfx_sentinel_main_tick_hook.data = gfx_window_head;
    Win32MainTickHook_strap(&gfx_sentinel_main_tick_hook);

    // Setup dummy user callback:
    gfx_user_callback_dummy.window_kill = OSWindow_kill_callback_stub;

    // Setup user callback:
    if (os_gfx_user_callback == 0)
    {
      gfx_user_callback = &gfx_user_callback_dummy;
    }
    else
    {
      gfx_user_callback = os_gfx_user_callback;

      if (gfx_user_callback->window_kill == 0)
      {
        gfx_user_callback->window_kill = gfx_user_callback_dummy.window_kill;
      }
    }

    result = (OSGraphicsSentinel*)gfx_window_roster;
  }
  else // !atom
  {
    // TODO : @LOG
  }

  return(result);
}


internal Win32MainTickHook *
GfxSentinel_main_tick_call(Win32MainTickHook *hook)
{
  // TODO : Assert(...);

  MSG msg;
  Umm slot = 0;
  GfxWindow *gfx_window = gfx_window_head;

  while(gfx_window != 0)
  {
    HWND ms_window_handle = gfx_window->ms_window_handle;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
      if (msg.message == WM_QUIT) {
        // TODO : signal
      } else {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }
    gfx_window = gfx_window->next;
  }

  return(hook->next);
}



// ==================== win32/gfx Window Management ====================

c_linkage OSWindow *
OSWindow_make(OSWindowSpec *spec, void *user_window_data, OSGraphicsBackend *os_gfx_backend)
{
  // TODO : Assert(...)
  OSWindow *result = 0;

  GfxBackend * backend_dispatch = (GfxBackend*)os_gfx_backend;
  GfxWindow *gfx_window = gfx_window_free;

  if (gfx_window != 0)
  {
    HWND ms_window_handle = CreateWindowExA(
      0,                                         // [in] DWORD dwExStyle,
      MAKEINTATOM(ms_general_window_class_atom), // [in, optional] LPCSTR lpClassName
      "win32_general_window",                    // [in, optional] LPCSTR lpWindowName
      WS_TILEDWINDOW,                            // [in] DWORD dwStyle
      CW_USEDEFAULT, CW_USEDEFAULT,              // [in] int X, Y screen positon
      spec->w, spec->h,                          // [in] nWidth, nHeight
      0, 0,                                      // [in, optional] HWND hWndParent, HMENU hMenu
      win32_main_module_handle,                  // [in, optional] HINSTANCE hInstance
      0                                          // [in, optional] LPVOID lpParam
    );

    if (ms_window_handle != NULL)
    {
      GfxWindowBackend * backend_data = backend_dispatch->window_make(spec);
      if (backend_data != 0)
      {
        gfx_window->backend_dispatch = backend_dispatch;
        gfx_window->backend_data = backend_data;
        gfx_window->ms_window_handle = ms_window_handle;
        gfx_window->spec = *spec;
        gfx_window->is_valid = 1;

        // Update window listings:
        gfx_window_free = gfx_window_free->next;
        gfx_window_free->prev = 0;

        gfx_window->prev = 0;
        gfx_window->next = gfx_window_head;

        if (gfx_window_head != 0)
        {
          gfx_window_head->prev = gfx_window;
        }
        gfx_window_head = gfx_window;

        // Setup user window data:
        gfx_window->user_data = user_window_data;

        // Display window:
        ShowWindow(ms_window_handle, SW_SHOW); 
        UpdateWindow(ms_window_handle); 

        result = (OSWindow*)gfx_window;
      } // !backend_data
    } // !ms_window_handle
  } // !window

  return(result);
}


c_linkage B8
OSWindow_kill(OSWindow *os_window)
{
  // TODO : Assert(...)
  B8 result = 0;

  GfxWindow * gfx_window = (GfxWindow*)os_window;

  if (gfx_window->is_valid)
  {
    if (gfx_window->backend_dispatch->window_kill(gfx_window->backend_data) != 0)
    {
        gfx_window->is_valid = 0;
        gfx_window->backend_data = 0;

      if (DestroyWindow(gfx_window->ms_window_handle) != 0)
      {
        // Update window listings:
        if (gfx_window == gfx_window_head)
        {
          gfx_window_head = gfx_window_head->next;
        }
        else
        {
          gfx_window->prev->next = gfx_window->next;
        }
        if (gfx_window->next != 0)
        {
            gfx_window->next->prev = gfx_window->prev;
        }

        gfx_window->prev = 0;
        gfx_window->next = gfx_window_free;

        if (gfx_window_free != 0)
        {
          gfx_window_free->prev = gfx_window;
        }
        gfx_window_free = gfx_window;

        gfx_user_callback->window_kill(os_window, gfx_user_callback->data, gfx_window->user_data);

        result = 1;
      }
      else
      {
        // TODO : @LOG
      }
    } // !backend_dispatch->window_kill
  } // !window->is_valid

  return(result);
}


internal LRESULT CALLBACK
MSWindowProcedure_default(HWND ms_window_handle, UINT message, WPARAM param_hi, LPARAM param_lo)
{
  LRESULT result = 0; // 0 = Generally ok "message handled" response.

  switch (message)
  {
    case WM_NCCREATE: {
      result = 1;
    } break;

    case WM_CREATE: {
      result = 0;
    } break;

    //case WM_NCACTIVATE

    case WM_CLOSE: {
      // NOTE : Temp window (e.g. one created for loading opengl extensions) may not be in the roster.

      GfxWindow * gfx_window = GfxWindow_from_hwnd(ms_window_handle);
      if (gfx_window != 0)
      {
        OSWindow_kill(gfx_window);
      }

      result = 0;
    } break;

    case WM_DESTROY: {
      PostQuitMessage(0); // Exitcode (0 = ok).
    } break;

    //case WM_NCDESTROY: { result = 0; } break;

    // case WM_SIZE: // Handle window resizing

    //
    // Painting and Drawing Messages:
    //

    case WM_PAINT: {
      PAINTSTRUCT paintstruct;
      // BeginPaint(...) validates entire client area.
      HDC device_context_handle = BeginPaint(ms_window_handle, &paintstruct);
      EndPaint(ms_window_handle, &paintstruct); 
      result = 0;
    } break;

    //case WM_NCPAINT: // Sent when window frame must be painted.
    //case WM_ERASEBKGND: // Sent when no default brush set in window class (client must draw).

    default: {
      result = DefWindowProcA(ms_window_handle, message, param_hi, param_lo);
    } break;

    // =============== INPUT DEVICE MESSAGES ===============
    // NOTE : Use raw-input instead.
    // ---------- MOUSE EVENTS ----------
    // ---------- KEYBOARD EVENTS ----------
  }

  return(result);
}


internal GfxWindow *
GfxWindow_from_hwnd(HWND ms_window_handle)
{
  GfxWindow *result = 0;

  for (GfxWindow *iter = gfx_window_head; iter != 0; iter = iter->next)
  {
    if (iter->ms_window_handle == ms_window_handle)
    {
      result = iter;
      break;
    }
  }

  return(result);
}



// ==================== Render Facility ====================

c_linkage B8
OSWindow_display_update(OSWindow *os_window)
{
  B8 result = 0;

  GfxWindow *gfx_window = (GfxWindow*)os_window;
    if (gfx_window->backend_dispatch->display_update(gfx_window) != 0)
    {
      result = 1;
    }

  return(result);
}

