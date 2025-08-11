#ifndef BMP_H
#define BMP_H

//#include "habit.h"


/******************************************************************************
 * BMP.H
 *
 * DESCRIPTION:
 *
 * FILE DATA FORMAT:
 * ----------------------------------------------------------------------------
 * | header          | File size, offs to image data, ...
 * ----------------------------------------------------------------------------
 * | dib header      | Image dimensions, ...
 * | aka info header | Multiple versions exists (detected by size)
 * ----------------------------------------------------------------------------
 * | Extra bit masks | 3 or 4 U32s, present if compression BI_ALPHA-/BITFIELDS.
 * ----------------------------------------------------------------------------
 * | color table     | Variable size, present if bpp <= 8.
 * ----------------------------------------------------------------------------
 * | gap1 optional   | Optional variable size structure alignment (artifact).
 * ----------------------------------------------------------------------------
 * | image data      | pixel array (rows padded to a multiple of 4B).
 * ----------------------------------------------------------------------------
 * | gap2 optional   | Optional variable size structure alignment (artifact).
 * ----------------------------------------------------------------------------
 * | color profile   | May be present if BITMAPV5HEADER used.
 * | optional        | Can be a filepath in which case it's loaded
 * | variable size   | in between color table and gap1.
 * ----------------------------------------------------------------------------
 *
******************************************************************************/


// ==================== BMP TYPE IDENTIFICATION ====================

// ---------- filename extensions ----------

enum BMP_FILENAME_EXTENSION {
  BMP_BMP = 0x504D42,
  BMP_DIB = 0x424944,
};


// ---------- bmp file header ----------
// Identify BMP/DIB via signature ('B','M' in little-endian)

enum BMP_FILE_SIGNATURE {
  BMP_BM = 0x4D42, // Windows 3.1x, 95, NT, ... etc.
  BMP_BA = 0x4142, // OS/2 struct bitmap array.
  BMP_CI = 0x4D49, // OS/2 struct color icon.
  BMP_CP = 0x4350, // OS/2 const color poS32er.
  BMP_IC = 0x4943, // OS/2 struct icon.
  BMP_PT = 0x5054, // OS/2 poS32er.
};


// ---------- bmp info header ----------
// NOTE: aka DIB header:
// Identified by size (some are undocumented / custom versions).
// There are 16b & 64b variants of BITMAP_CORE_HEADER_2.

enum BMP_DIB_HEADER_SIZE {
  // Original header and its versions.
  BITMAP_CORE_HEADER_SIZE      = 12,
  OS2_1X_BITMAP_HEADER_SIZE    = BITMAP_CORE_HEADER_SIZE,
  BITMAP_CORE_HEADER_2_SIZE    = 16,
  OS2_2X_BITMAP_HEADER_SIZE    = BITMAP_CORE_HEADER_2_SIZE,
  BITMAP_CORE_HEADER_2_V2_SIZE = 64,
  // Info header and its extensions:
  BITMAP_INFO_HEADER_SIZE      =  40,
  BITMAP_V2_INFO_HEADER_SIZE   =  52,
  BITMAP_V3_INFO_HEADER_SIZE   =  56,
  BITMAP_V4_HEADER_SIZE        = 108,
  BITMAP_V5_HEADER_SIZE        = 124,
};

enum BMP_COMPRESSION_METHOD {
  BMP_BI_RGB            =  0, // No compression - most common.
  BMP_BI_RLE8           =  1, // Only with 8-bit/pixel bitmaps.
  BMP_BI_RLE4           =  2, // Only with 4-bit/pixel bitmaps.
  BMP_BI_BITFIELDS      =  3, // OS22X = Huffman 1D, V2 & V3+ = RGB & RGBA bit field masks.
  BMP_BI_JPEG           =  4, // OS22X = RLE-24, V4+ = JPEG image for prS32ing.
  BMP_BI_PNG            =  5, // BITMAPV4INFOHEADER+: PNG image for prS32ing
  BMP_BI_ALPHABITFIELDS =  6, // RGBA bit field masks (only >= Win CE 5.0 with .NET 4.0)
  BMP_BI_CMYK           = 11, // No compression, only Windows Metafile CMYK
  BMP_BI_CMYKRLE8       = 12, // RLE-8,          only Windows Metafile CMYK
  BMP_BI_CMYKRLE4       = 13, // RLE-4,          only Windows Metafile CMYK
};



// ==================== BMP FILE FORMAT ====================

#pragma pack(push, 1)

// ---------- bitmap file header ----------

typedef struct {
  // File type ID field;
  U16 signature;
  U32 file_size;

  // Reserved variables - application specific:
  // Usually 0, propably for padding to byte boundary?
  // Both or second may be used by app?
  U16 reserved1;
  U16 reserved2;

  // Pixel array location in bytes from beginning of file.
  U32 data_offset;
} BitmapFileHeader;


// ---------- bitmap dib headers ----------

typedef struct {
  // Bitmap version depends on the size of this DIB header.
  U32 size_indicator;

  //// HEADER VERSIONS ////
  // NOTE : OG DIB Headers Not Implemented:
  // BITMAP_CORE_HEADER_2 / OS2_2X_BITMAP_HEADER
  // BITMAP_CORE_HEADER_2_V2
  // NOTE : Bitmap Info Header Extensions Not Implemented:
  // BITMAP_V2_INFO_HEADER
  // BITMAP_V3_INFO_HEADER

  union
  {
    //// BITMAP_CORE_HEADER / OS2_1X_BITMAP_HEADER ////
    struct {
      U16 bch_width;
      U16 bch_height;
      U16 bch_color_planes;
      U16 bch_bits_per_pixel;
    };

    //// BITMAP_INFO_HEADER & EXTENSIONS ////
    struct {
      struct {
        struct {

          // BITMAP_INFO_HEADER :

          // Decompressed image dimensions in pixels:
          // Top-down DIBs can't be compressed and have negative height, origin at top-left.
          S32 width;
          S32 height;
          U16 color_planes;   // Count used for target device, must be 1.
          U16 bits_per_pixel; // Pixel color depth
          U32 compression;    // Compresson method used.
          U32 image_size;     // If compressed, raw uncompressed data with padding.

          // Print resolution of image:
          S32 pixels_per_meterx;
          S32 pixels_per_metery;

          // Color table info (following this struct and its extensions):
          U32 color_palette;    // Number of colors in palette.
          U32 important_colors; // Number of those colors used, 0 = all.
        };

        // BITMAP_V4_HEADER :

        // Color channel masks:
        U32 channel_bitmask_red;
        U32 channel_bitmask_green;
        U32 channel_bitmask_blue;
        U32 channel_bitmask_alpha;

        // Color space of DIB:
        U32 color_space_type;

        // Color space endpoints:
        U32 endpoint_coordx_red;
        U32 endpoint_coordy_red;
        U32 endpoint_coordz_red;
        U32 endpoint_coordx_green;
        U32 endpoint_coordy_green;
        U32 endpoint_coordz_green;
        U32 endpoint_coordx_blue;
        U32 endpoint_coordy_blue;
        U32 endpoint_coordz_blue;

        // color channel gammas:
        U32 channel_gamma_red;
        U32 channel_gamma_green;
        U32 channel_gamma_blue;
      };

      // BITMAP_V5_HEADER :

      U32 intent;
      U32 icc_profile_data; // offset from the beginning of V5 struct
      U32 icc_profile_size;
      U32 reserved; // = 0
    };
  };

} BitmapDIBHeader;


// ---------- bitmap file ----------

typedef struct {
  BitmapFileHeader header;
  BitmapDIBHeader  info;
  // Note: This structure is followed by a color-table if such is used.
} BitmapFile;


#pragma pack(pop, 1)

#endif // ! BMP_H
