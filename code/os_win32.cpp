//#include "os_win32.h"

// ==================== GLOBALS ====================

HMODULE win32_main_module_handle = 0;

// string handling:
U8     win32_filepath_directory_separator = '\\';
U8     win32_main_module_filepath[255] = {0};
U16    win32_main_module_filepath_len = 0;

Win32MainTickHook win32_main_tick_hooks_head = {0};



// ==================== OS Context ====================

c_linkage OSContext *
OS_init(OSContext *os_ctx_mem)
{
  OSContext * result = &win32_main_module_handle;

  // GetCurrentProcess()
  win32_main_module_handle = GetModuleHandle(NULL);
  win32_main_module_filepath_len = GetModuleFileNameA(win32_main_module_handle,
                                                      (LPSTR)win32_main_module_filepath,
                                                      sizeof(win32_main_module_filepath));

  if (win32_main_module_filepath_len == 0 ||
    win32_main_module_filepath_len == sizeof(win32_main_module_filepath))
  {
    // TODO : @LOG
  }

  return(result);
}


// ---------- run tree ----------

c_linkage U16
OSContext_get_mainproc_fullpath(void **path_ptr_receiver, OSContext *os_ctx)
{
  // MS-stdlib crt _get_pgmptr/wpgmptr return path to executable file.
  *path_ptr_receiver = (void*)win32_main_module_filepath;
  return(win32_main_module_filepath_len);
}


// ==================== Program Loop ====================

c_linkage B8
OS_main_tick(OSContext *os_ctx)
{
  B8 result = 1;

  Win32MainTickHook *iter = win32_main_tick_hooks_head.next;
  while (iter != 0)
  {
    iter = iter->call(iter);
  }

  return(result);
}



// ==================== Win32 Internals ====================

// ---------- program loop hooks ----------

c_linkage void
Win32MainTickHook_strap(Win32MainTickHook *hook)
{
  hook->next = win32_main_tick_hooks_head.next;
  win32_main_tick_hooks_head.next = hook;
}


c_linkage void
Win32MainTickHook_sever(Win32MainTickHook *hook)
{
  Win32MainTickHook **iter = &win32_main_tick_hooks_head.next;
  while (*iter != hook)
  {
    iter = &(*iter)->next;
  }
  *iter = (*iter)->next;
}



// ==================== os - Memory Management ====================

// ---------- common tools ----------

c_linkage void OSMemory_zero(void *data, Umm size)
{
  ZeroMemory((PVOID)data, (SIZE_T)size);
}


c_linkage void OSMemory_copy(void *dst, void *src, Umm size)
{
  MoveMemory((PVOID)dst, (VOID*)src, (SIZE_T)size);
}


// ---------- os memory info ----------

c_linkage OSMemoryScheme
OSMemoryScheme_inquire(void)
{
  OSMemoryScheme result = {0};

  SYSTEM_INFO si;
  GetNativeSystemInfo(&si);
  DWORD allocation_granularity = si.dwAllocationGranularity;
  DWORD page_small = si.dwPageSize;
  DWORD page_large = 0;

  result.alignments.page_small = (Umm)page_small;
  result.alignments.page_large = (Umm)page_large;
  result.alignments.allocation_granularity = (Umm)allocation_granularity;
  result.page_properties = 0;
  result.page_state = OSMEM_RESERVED | OSMEM_COMMITTED | OSMEM_FORCE_COMMITTED | OSMEM_DECOMMITTED | OSMEM_RELEASED;

  return (result);
}


// ---------- os memory block ----------

c_linkage U8 *
OSMemoryBlock_request(U8 *base, Umm size, Umm alignment,
                      OSMemoryPageProperties properties,
                      OSMemoryPageState state)
{
  // TODO : Assert(...);

  U8 *result = 0;

  // Setup virtual memory call params:
  DWORD operation_flags = 0;
  DWORD protect_flags = PAGE_READWRITE;

  switch(state)
  {
    case OSMEM_RESERVED :
    {
      operation_flags |= MEM_RESERVE;
      result = (U8 *)VirtualAlloc((LPVOID)base, (SIZE_T)size, operation_flags, protect_flags);
    } break;

    case OSMEM_COMMITTED:
    case OSMEM_FORCE_COMMITTED:
    {
      operation_flags |= MEM_COMMIT;
      result = (U8 *)VirtualAlloc((LPVOID)base, (SIZE_T)size, operation_flags, protect_flags);
    } break;

    case OSMEM_DECOMMITTED:
    {
      operation_flags |= MEM_DECOMMIT;
      if (VirtualFree((LPVOID)base, size, operation_flags) != 0)
      {
        result = base;
      }
    } break;

    case OSMEM_RELEASED:
    {
      operation_flags |= MEM_RELEASE;
      if (VirtualFree((LPVOID)base, 0, operation_flags) != 0)
      {
        result = base;
      }
    } break;

    default :
    {
      // TODO : Assert(...);
    } break;
  }

  return(result);
}



// ==================== os - File I/O ====================

c_linkage OSFile *
OSFile_make(void *fullpath, OSFileModeFlags flags)
{
  OSFile *result = 0;

  //// Setup win32 fopen params ////

  DWORD dwDesiredAccess = 0;
  // GENERIC_EXECUTE
  if (flags & OSFILE_READ)
  {
    dwDesiredAccess |= GENERIC_READ;
  }
  if (flags & OSFILE_CHANGE || flags & OSFILE_APPEND)
  {
    dwDesiredAccess |= GENERIC_WRITE;
  }

  DWORD dwCreationDisposition = 0;
  if (flags & OSFILE_CREATE)
  {
    dwCreationDisposition |= CREATE_NEW;
  }
  else
  {
    dwCreationDisposition |= OPEN_EXISTING;
  }


  //// Open file ////

  HANDLE file_handle = CreateFileA(
    (LPCSTR)fullpath,
    dwDesiredAccess,
    FILE_SHARE_READ|FILE_SHARE_WRITE,//FILE_SHARE_DELETE [in] dwShareMode(simultaneous opens).
    NULL, // NULL = [in, opt] lpSecurityAttributes
    dwCreationDisposition,
    FILE_ATTRIBUTE_NORMAL, // FILE_FLAG_OVERLAPPED (async-io) [in] dwFlagsAndAttributes.
    NULL // [in, opt] HANDLE hTemplateFile.
  );

  //// Handle errors ////

  if (file_handle == INVALID_HANDLE_VALUE)
  {
    // TODO : @LOG
  }

  //// Fill result ////

  result = (OSFile*)file_handle;

  return(result);
}


c_linkage B8
OSFile_cull(void * fullpath)
{
  B8 result = 0;

  if (DeleteFileA((LPCSTR) fullpath) != 0)
  {
    result = 1;
  }

  return(result);
}


c_linkage B8
OSFile_kill(OSFile * os_file)
{
  B8 result = 0;
  HANDLE file_handle = (HANDLE)os_file;

  if (CloseHandle(file_handle) != 0)
  {
    result = 1;
  }

  return(result);
}


c_linkage U64
OSFile_size(OSFile *os_file, U64 size)
{
  U64 result = 0;
  HANDLE file_handle = (HANDLE)os_file;

  if (size == -1)
  {
    if (GetFileSizeEx(file_handle, (PLARGE_INTEGER)&result) == 0)
    {
      result = -1;
      // TODO : @LOG
    }
  }
  else
  {
    LONG* offset = (LONG*)&size;
    LONG  offs_lo = offset[0];
    PLONG offs_hi = &offset[1];

    if (SetFilePointer(file_handle, offs_lo, (PLONG)&offs_hi, FILE_BEGIN) != 0)
    {
      if (SetEndOfFile(file_handle) != 0)
      {
        result = size;
      }
      else
      {
        // TODO : @LOG
      }
    }
    else
    {
      // TODO : @LOG
    }
  }

  return(result);
}


c_linkage Umm
OSFile_read(OSFile *os_file, void *dst, Umm offs, Umm size)
{
  // TODO : handle large files

  Umm result = -1;
  HANDLE file_handle = (HANDLE)os_file;

  if (size > 0)
  {
    LONG* offset = (LONG*)&offs;
    LONG  offs_lo = offset[0];
    PLONG offs_hi = &offset[1];

    if (SetFilePointer(file_handle, offs_lo, (PLONG)&offs_hi, FILE_BEGIN) != 0)
    {
      DWORD bytes_read = 0;
      if (ReadFile(file_handle, dst, (DWORD)size, &bytes_read, NULL) != 0)
      {
        result = (Umm)bytes_read;
      }
      else
      {
        // TODO : @LOG
      }
    }
    else
    {
      // TODO: @LOG
    }
  } // ! size > 0
  else
  {
    result = 0;
  }

  return(result);
}


c_linkage Umm
OSFile_write(OSFile *os_file, void *src, Umm offs, Umm size)
{
  // TODO : handle large files

  Umm result = -1;
  HANDLE file_handle = (HANDLE)os_file;

  if (size > 0)
  {
    LONG* offset = (LONG*)&offs;
    LONG  offs_lo = offset[0];
    PLONG offs_hi = &offset[1];

    if (SetFilePointer(file_handle, offs_lo, (PLONG)&offs_hi, FILE_BEGIN) != 0)
    {
      DWORD bytes_read = 0;
      if (WriteFile(file_handle, (LPCVOID)src, (DWORD)size, &bytes_read, NULL) != 0)
      {
        // NULL = [in, out, optional] lpOverlapped (async)
        result = (Umm)bytes_read;
      }
      else
      {
        // TODO: @LOG
      }
    }
    else
    {
      // TODO: @LOG
    }
  } // ! size > 0
  else
  {
    result = 0;
  }

  return(result);
}

