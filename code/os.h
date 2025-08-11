#ifndef OS_H
#define OS_H

//#include "habit.h"

/******************************************************************************
 * OS.H
 *
 * DESCRIPTION:
 * Fundamental procedures which don't require backing memory.
 *
 * USAGE:
 * os_.h files provide an abstraction interface.
 * >> typedef Foo_Init(Bar bar); // Provided function signature.
 * >> static  Foo_init(Bar bar); // Declaration in [impl].h.
 * >> Foo_init(bar);             // User code.
 *
 * TODO :
 * Precision timers.
 * DLL handling.
 *
******************************************************************************/


// ==================== OS Context ====================
// Holds basic info such as process identification
// which is passed to extensions such as the os graphics layer.

//#define OS_CONTEXT_ALLOC_SIZE // Impl -> 0 if user doesn't provide memory.

typedef void OSContext;

typedef OSContext OS_Init(OSContext *os_ctx_mem);
// Returns 0 on failure.
// impl -> return >0 if not using context.


// ---------- run tree ----------

//#define OS_DIRECTORY_SEPARATOR "as_literal_string"

typedef U16 OSContext_Get_Mainproc_Fullpath(void **path_ptr_receiver, OSContext *os_ctx);
// Impl -> Returns string size without null terminator and sets given ptr.
// User -> Failure if both size == 0 and given ptr == 0.



// ==================== Program Loop ====================

//#define PROGRAM_ENTRYPOINT          // includes {
//#define PROGRAM_EXITPOINT(exitcode) // includes }

typedef B8 OS_Main_Tick(OSContext *os_ctx);
// Returns 0 on termination request.
// User -> Call at beginning of each program main loop iteration.



// ==================== Memory Management ====================

// ---------- common tools ----------

typedef void OSMemory_Zero(void *data, Umm size);
typedef void OSMemory_Copy(void *dst, void *src, Umm size); // Equivalent to crt move.


// ---------- memory info ----------

typedef struct {
  Umm page_small; // Generally 4KB
  Umm page_large; // Set 0 if not supported.
  Umm allocation_granularity; // Generally 64KB
} OSMemoryAlignments;


typedef U8 OSMemoryPageProperties;
enum OS_MEMORY_PAGE_PROPERTIES {
  OSMEM_NONE = 0,
  //OSMEM_NO_READ =        // Prevent reading.
  //OSMEM_NO_WRITE     = , // Prevent writing.
  //OSMEM_FAULT_SIGNAL = , // aka protect - Raise page fault on access.
  //OSMEM_PREVENT_SWAP = , // aka lock - No swapping to page file.
  //OSMEM_RUNNABLE     = , // aka execute - machine-code block.
};


typedef U8 OSMemoryPageState;
enum OS_MEMORY_PAGE_STATE {
  //
  // RESERVE-COMMIT BEHAVIOUR:
  //
  // If supporting manual reserve-commit allocation scheme:
  // Set OSMEM_RESERVED | _COMMITTED | _FORCE_COMMITTED | _DECOMMITTED | _RELEASED.
  //
  // If committing on page access:
  // - Set OSMEM_RESERVED | _FORCE_COMMITTED | _RELEASED
  // - OSMEM_FORCE_COMMITTED used to iterate over pages on commit.
  //
  // If no support for reserve-commit style memory operations:
  // - Set OSMEM_RESERVED | OSMEM_RELEASED
  //
  OSMEM_RESERVED = 1,
  OSMEM_COMMITTED,
  OSMEM_FORCE_COMMITTED,
  OSMEM_DECOMMITTED,
  OSMEM_RELEASED,
};


typedef struct {
  OSMemoryAlignments alignments;
  OSMemoryPageProperties page_properties;
  OSMemoryPageState page_state;
} OSMemoryScheme;



// ---------- memory functions ----------

typedef OSMemoryScheme OSMemoryScheme_Inquire(void);

typedef U8 * OSMemoryBlock_Request(U8 *base, Umm size, Umm alignment,
                                   OSMemoryPageProperties properties,
                                   OSMemoryPageState state);
// A memory blocks represents a range of pages with identical configuration.
// User/Impl -> *base ignored if adressing not supported, 0 for os to decide location.
// User/Impl -> alignment ignored if only 1 allocation_granularity & page_size for reserves/commits.



// ==================== File I/O ====================

typedef void OSFile;
//#define OSFILE_INVALID // impl -> failure value returned by _open.

typedef U8 OSFileModeFlags;
enum OS_FILE_MODE_FLAGS {
  // File creation:
  OSFILE_OPEN   = 0,
  OSFILE_CREATE = 1, // Create new file.
  // File access:
  OSFILE_READ    = 4,
  OSFILE_CHANGE  = 8, // Write data within file.
  OSFILE_APPEND  = 16, // (e.g. a database may only allow append).
  //OSFILE_EXECUTE,
  // Following processes rights:
  //OSFILE_SHARE_READ,
  //OSFILE_SHARE_WRITE,
  //OSFILE_SHARE_DELETE,
};

// Open/Create file - returns file-handle, check validity (handle != OSFILE_INVALID).
typedef OSFile * OSFile_Make(void *fullpath, OSFileModeFlags flags);

// Delete file - returns 0 on failure.
typedef B8 OSFile_Cull(void *fullpath);

// Close file-handle - returns 0 on failure.
typedef B8 OSFile_Kill(OSFile * os_file);

// File size : get with size = -1, set with size != -1.
// Returns -1 on failure.
typedef U64 OSFile_Size(OSFile *os_file, U64 size);

// Returns number of bytes read, failure = -1.
typedef Umm OSFile_Read(OSFile *os_file, void *dst, Umm offs, Umm size);

// Returns number of bytes written, failure = -1.
typedef Umm OSFile_Write(OSFile *os_file, void *src, Umm offs, Umm size);

// Returns number of bytes written, failure = -1.
//typedef Umm OSFile_append(OSFile *os_file, void *src, Umm size);

#endif // !OS_H
