#ifndef SORE_H
#define SORE_H

//#include "vecmat__.h"
//#include "memory.h"
//#include "bmp.h"

/******************************************************************************
 * SORE.H
 *
 * !UNDER CONSTRUCTION!
 *
 * DESCRIPTION:
 * Simple software renderer.
 *
 * COLOR MODEL
 * Internal pixel format is little-endian RGBA32 with layout 0xAABBGGRR.
 *
 * TODO :
 * Graphics Backends
 *  Attach to software backend.
 *  Pipe-through graphics accelerated backend.
 * PIXEL HANDLING:
 *   RGBA32 layout conversions.
 *   Pixel Un-/Packing.
 *   Gamma Correction.
 *   Color Mapping.
 *   Composition/Blending.
 *   Transfer Functions.
 * RENDERING:
 *   Rasterizer
 *     Coordinate Spaces (world, object, camera).
 *     Primitive Construction (parallelograms).
 *     Pixel Coverage.
 *     Grouping (simd computations).
 *     Threading (sub-divide canvas).
 *   Sampling
 *     NearestNeighbour/Bilinear Filtering etc...
 *
******************************************************************************/


// ==================== PIXEL STRUCTURES ====================

// ---------- Color Independent Framebuffer ----------

typedef struct {
  U8 *data;
  S32 w;
  S32 h;
  S32 pitch; // Bytes per row with padding.
} PixelBuffer;


// ==================== TEMPORARY ====================

// Returns 0 on failure.
c_linkage B8 SORE_load_bitmap(MemoryArena *arena, PixelBuffer * receiver, String8 *fullpath)
{
  // TODO : Assert(...);

  B8 result = 0;
  OSFile * file_handle = OSFile_make(fullpath->data, OSFILE_READ | OSFILE_CHANGE | OSFILE_APPEND);

  BitmapFile *bmp_file;
  Umm bmp_read;

  U8 *bmp_pixels;
  S32 bmp_width;
  S32 bmp_height;
  U32 bmp_size;
  S32 bmp_pitch;

  if (file_handle != OSFILE_INVALID) {
    U32 error = 0;

    U64 file_size = OSFile_size(file_handle, -1);
    if (file_size == -1) { error = 1; }

    if (!error) {
      // Load bmp file headers to end of alloc, pixel data at front.

      bmp_pixels = MemoryArena_push_size(arena, file_size, 1); 
      bmp_file   = (BitmapFile*)bmp_pixels + file_size - sizeof(BitmapFile);
      bmp_read   = OSFile_read(file_handle, bmp_file, 0, sizeof(BitmapFile));

      if (bmp_pixels == 0 || bmp_read != sizeof(BitmapFile)) { error = 1; }
    }

    if (!error) {
      //////////////////// Grab BMP info:

      if (bmp_file->info.size_indicator >= BITMAP_INFO_HEADER_SIZE )
      {
        bmp_width  = bmp_file->info.width;
        bmp_height = bmp_file->info.height;
        bmp_size   = bmp_file->info.image_size;
      }
      else
      {
        bmp_width  = bmp_file->info.bch_width;
        bmp_height = bmp_file->info.bch_height;
        bmp_size   = bmp_width + bmp_height * bmp_file->info.bch_bits_per_pixel;
        // TODO : bbp is total px size, not color plane ?
      }
      bmp_pitch = Align((Smm)bmp_width * 4, 4);

      // Read-in pixel data:
      bmp_read = OSFile_read(file_handle, bmp_pixels, bmp_file->header.data_offset, bmp_size);

      if (bmp_read != bmp_size) { error = 1; }
    }

    //////////////////// Cleanup:
    if (!error) {
      result = 1;

      receiver->data   = bmp_pixels;
      receiver->w      = bmp_width;
      receiver->h      = bmp_height;
      receiver->pitch  = bmp_pitch;

      MemoryArena_pop_to(arena, bmp_pixels + bmp_size);
    }
    else
    {
      MemoryArena_pop_to(arena, bmp_pixels);
    }

    OSFile_kill(file_handle);
  }

  return(result);
}


#endif // ! SORE_H
