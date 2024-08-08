/****************************************************************************
*
*    Copyright (c) 2005 - 2019 by Orbita Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Orbita Corporation. This is proprietary information owned by
*    Orbita Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Orbita Corporation.
*
*****************************************************************************/


#ifndef _VIV_DC_UTIL_H_
#define _VIV_DC_UTIL_H_

#include "viv_dc_type.h"
#include "viv_dc_core.h"

#define gcd2D_COMPRESSION_DEC400_ALIGN_MODE_32_BYTES 32
typedef enum _vivCOMPRESSION_DEC400_ALIGN_MODE
{
    vivCOMPRESSION_DEC400_16_BYTES_ALIGNED = 0,
    vivCOMPRESSION_DEC400_32_BYTES_ALIGNED,
    vivCOMPRESSION_DEC400_64_BYTES_ALIGNED,
} vivCOMPRESSION_DEC400_ALIGN_MODE;

typedef gctUINT   gctUINT;
typedef gctINT    gctBOOL;
typedef gctINT    gctINT;
typedef gctPOINTER gctPOINTER;
typedef gctUINT   gceSURF_FORMAT;

#define gcvFALSE 0
#define gcvTRUE  1

#define gcdPI                   3.14159265358979323846f

#define gcoMATH_Sine(X)       (gctFLOAT)(sinf((X)))

#define vivMATH_Add(X, Y)          (gctFLOAT)((X) + (Y))
#define vivMATH_Multiply(X, Y)     (gctFLOAT)((X) * (Y))
#define vivMATH_Divide(X, Y)       (gctFLOAT)((X) / (Y))
#define vivMATH_DivideFromUInteger(X, Y) (gctFLOAT)(X) / (gctFLOAT)(Y)

#define vivMATH_Int2Float(X)   (gctFLOAT)(X)

static gctBOOL Features[gcvFEATURE_COUNT];

/********************************* BMP header ******************************/
#define BF_TYPE 0x4D42             /* "MB" */

#pragma pack(1)
/**** BMP file header structure ****/
typedef struct BMPFILEHEADER {
    unsigned short bfType;           /* Magic number for file */
    unsigned int   bfSize;           /* Size of file */
    unsigned short bfReserved1;      /* Reserved */
    unsigned short bfReserved2;      /* ... */
    unsigned int   bfOffBits;        /* Offset to bitmap data */
} BMPFILEHEADER;

/**** BMP file info structure ****/
typedef struct BMPINFOHEADER {
    unsigned int   biSize;           /* Size of info header */
    int            biWidth;          /* Width of image */
    int            biHeight;         /* Height of image */
    unsigned short biPlanes;         /* Number of color planes */
    unsigned short biBitCount;       /* Number of bits per pixel */
    unsigned int   biCompression;    /* Type of compression to use */
    unsigned int   biSizeImage;      /* Size of image data */
    int            biXPelsPerMeter;  /* X pixels per meter */
    int            biYPelsPerMeter;  /* Y pixels per meter */
    unsigned int   biClrUsed;        /* Number of colors used */
    unsigned int   biClrImportant;   /* Number of important colors */
} BMPINFOHEADER;

/*
 * Constants for the biCompression field...
 */

#define BIT_RGB       0             /* No compression - straight BGR data */
#define BIT_RLE8      1             /* 8-bit run-length compression */
#define BIT_RLE4      2             /* 4-bit run-length compression */
#define BIT_BITFIELDS 3             /* RGB bitmap with RGB masks */

/**** Colormap entry structure ****/
typedef struct RGB {
    unsigned char   rgbBlue;          /* Blue value */
    unsigned char   rgbGreen;         /* Green value */
    unsigned char   rgbRed;           /* Red value */
    unsigned char   rgbReserved;      /* Reserved */
} RGB;

/**** Bitmap information structure ****/
typedef struct _BMPINFO {
    BMPINFOHEADER   bmiHeader;      /* Image header */
    union {
    RGB             bmiColors[256]; /* Image colormap */
    gctUINT         mask[3];        /* RGB masks */
    };
} BMPINFO;
#pragma pack()

static gctUINT32 lut1[] = {0x00ffffff, 0x00000000};

static gctUINT32 lut2[] = {0x00000000, 0x00404040, 0x00808080, 0x00FFFFFF};

static gctUINT32 lut4[] = {
    0x00000000, 0x00c0dcc0, 0x00006000, 0x00008000,
    0x0000c000, 0x00000040, 0x00004040, 0x0000a040,
    0x0000e040, 0x00002080, 0x00006080, 0x0000c080,
    0x0000e080, 0x000020c0, 0x000080c0, 0x00ffffff,
};

static gctUINT32 lut[] = {
0x00000000, 0x00800000, 0x00008000, 0x00808000, 0x00000080, 0x00800080, 0x00008080, 0x00c0c0c0,
0x00c0dcc0, 0x00a6caf0, 0x00402000, 0x00602000, 0x00802000, 0x00a02000, 0x00c02000, 0x00e02000,
0x00004000, 0x00204000, 0x00404000, 0x00604000, 0x00804000, 0x00a04000, 0x00c04000, 0x00e04000,
0x00006000, 0x00206000, 0x00406000, 0x00606000, 0x00806000, 0x00a06000, 0x00c06000, 0x00e06000,
0x00008000, 0x00208000, 0x00408000, 0x00608000, 0x00808000, 0x00a08000, 0x00c08000, 0x00e08000,
0x0000a000, 0x0020a000, 0x0040a000, 0x0060a000, 0x0080a000, 0x00a0a000, 0x00c0a000, 0x00e0a000,
0x0000c000, 0x0020c000, 0x0040c000, 0x0060c000, 0x0080c000, 0x00a0c000, 0x00c0c000, 0x00e0c000,
0x0000e000, 0x0020e000, 0x0040e000, 0x0060e000, 0x0080e000, 0x00a0e000, 0x00c0e000, 0x00e0e000,
0x00000040, 0x00200040, 0x00400040, 0x00600040, 0x00800040, 0x00a00040, 0x00c00040, 0x00e00040,
0x00002040, 0x00202040, 0x00402040, 0x00602040, 0x00802040, 0x00a02040, 0x00c02040, 0x00e02040,
0x00004040, 0x00204040, 0x00404040, 0x00604040, 0x00804040, 0x00a04040, 0x00c04040, 0x00e04040,
0x00006040, 0x00206040, 0x00406040, 0x00606040, 0x00806040, 0x00a06040, 0x00c06040, 0x00e06040,
0x00008040, 0x00208040, 0x00408040, 0x00608040, 0x00808040, 0x00a08040, 0x00c08040, 0x00e08040,
0x0000a040, 0x0020a040, 0x0040a040, 0x0060a040, 0x0080a040, 0x00a0a040, 0x00c0a040, 0x00e0a040,
0x0000c040, 0x0020c040, 0x0040c040, 0x0060c040, 0x0080c040, 0x00a0c040, 0x00c0c040, 0x00e0c040,
0x0000e040, 0x0020e040, 0x0040e040, 0x0060e040, 0x0080e040, 0x00a0e040, 0x00c0e040, 0x00e0e040,
0x00000080, 0x00200080, 0x00400080, 0x00600080, 0x00800080, 0x00a00080, 0x00c00080, 0x00e00080,
0x00002080, 0x00202080, 0x00402080, 0x00602080, 0x00802080, 0x00a02080, 0x00c02080, 0x00e02080,
0x00004080, 0x00204080, 0x00404080, 0x00604080, 0x00804080, 0x00a04080, 0x00c04080, 0x00e04080,
0x00006080, 0x00206080, 0x00406080, 0x00606080, 0x00806080, 0x00a06080, 0x00c06080, 0x00e06080,
0x00008080, 0x00208080, 0x00408080, 0x00608080, 0x00808080, 0x00a08080, 0x00c08080, 0x00e08080,
0x0000a080, 0x0020a080, 0x0040a080, 0x0060a080, 0x0080a080, 0x00a0a080, 0x00c0a080, 0x00e0a080,
0x0000c080, 0x0020c080, 0x0040c080, 0x0060c080, 0x0080c080, 0x00a0c080, 0x00c0c080, 0x00e0c080,
0x0000e080, 0x0020e080, 0x0040e080, 0x0060e080, 0x0080e080, 0x00a0e080, 0x00c0e080, 0x00e0e080,
0x000000c0, 0x002000c0, 0x004000c0, 0x006000c0, 0x008000c0, 0x00a000c0, 0x00c000c0, 0x00e000c0,
0x000020c0, 0x002020c0, 0x004020c0, 0x006020c0, 0x008020c0, 0x00a020c0, 0x00c020c0, 0x00e020c0,
0x000040c0, 0x002040c0, 0x004040c0, 0x006040c0, 0x008040c0, 0x00a040c0, 0x00c040c0, 0x00e040c0,
0x000060c0, 0x002060c0, 0x004060c0, 0x006060c0, 0x008060c0, 0x00a060c0, 0x00c060c0, 0x00e060c0,
0x000080c0, 0x002080c0, 0x004080c0, 0x006080c0, 0x008080c0, 0x00a080c0, 0x00c080c0, 0x00e080c0,
0x0000a0c0, 0x0020a0c0, 0x0040a0c0, 0x0060a0c0, 0x0080a0c0, 0x00a0a0c0, 0x00c0a0c0, 0x00e0a0c0,
0x0000c0c0, 0x0020c0c0, 0x0040c0c0, 0x0060c0c0, 0x0080c0c0, 0x00a0c0c0, 0x00fffbf0, 0x00a0a0a4,
0x00808080, 0x00ff0000, 0x0000ff00, 0x00ffff00, 0x000000ff, 0x00ff00ff, 0x0000ffff, 0x00ffffff,
};

static gctUINT32 horKernel[] =
{
    0x00000000, 0x20000000, 0x00002000, 0x00000000, 0x00000000, 0x00000000, 0x23fd1c03, 0x00000000,
    0x00000000, 0x00000000, 0x181f0000, 0x000027e1, 0x00000000, 0x00000000, 0x00000000, 0x2b981468,
    0x00000000, 0x00000000, 0x00000000, 0x10f00000, 0x00002f10, 0x00000000, 0x00000000, 0x00000000,
    0x32390dc7, 0x00000000, 0x00000000, 0x00000000, 0x0af50000, 0x0000350b, 0x00000000, 0x00000000,
    0x00000000, 0x3781087f, 0x00000000, 0x00000000, 0x00000000, 0x06660000, 0x0000399a, 0x00000000,
    0x00000000, 0x00000000, 0x3b5904a7, 0x00000000, 0x00000000, 0x00000000, 0x033c0000, 0x00003cc4,
    0x00000000, 0x00000000, 0x00000000, 0x3de1021f, 0x00000000, 0x00000000, 0x00000000, 0x01470000,
    0x00003eb9, 0x00000000, 0x00000000, 0x00000000, 0x3f5300ad, 0x00000000, 0x00000000, 0x00000000,
    0x00480000, 0x00003fb8, 0x00000000, 0x00000000, 0x00000000, 0x3fef0011, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00004000, 0x00000000, 0x00000000, 0x00000000, 0x20002000, 0x00000000,
    0x00000000, 0x00000000, 0x1c030000, 0x000023fd, 0x00000000, 0x00000000, 0x00000000, 0x27e1181f,
    0x00000000, 0x00000000, 0x00000000, 0x14680000, 0x00002b98, 0x00000000, 0x00000000, 0x00000000,
    0x2f1010f0, 0x00000000, 0x00000000, 0x00000000, 0x0dc70000, 0x00003239, 0x00000000, 0x00000000,
    0x00000000, 0x350b0af5, 0x00000000, 0x00000000, 0x00000000, 0x087f0000, 0x00003781, 0x00000000,
    0x00000000, 0x00000000, 0x399a0666, 0x00000000, 0x00000000, 0x00000000, 0x04a70000, 0x00003b59,
    0x00000000, 0x00000000, 0x00000000, 0x3cc4033c, 0x00000000, 0x00000000, 0x00000000, 0x021f0000,
};

static gctUINT32 verKernel[] =
{
    0x00000000, 0x20000000, 0x00002000, 0x00000000, 0x00000000, 0x00000000, 0x23fd1c03, 0x00000000,
    0x00000000, 0x00000000, 0x181f0000, 0x000027e1, 0x00000000, 0x00000000, 0x00000000, 0x2b981468,
    0x00000000, 0x00000000, 0x00000000, 0x10f00000, 0x00002f10, 0x00000000, 0x00000000, 0x00000000,
    0x32390dc7, 0x00000000, 0x00000000, 0x00000000, 0x0af50000, 0x0000350b, 0x00000000, 0x00000000,
    0x00000000, 0x3781087f, 0x00000000, 0x00000000, 0x00000000, 0x06660000, 0x0000399a, 0x00000000,
    0x00000000, 0x00000000, 0x3b5904a7, 0x00000000, 0x00000000, 0x00000000, 0x033c0000, 0x00003cc4,
    0x00000000, 0x00000000, 0x00000000, 0x3de1021f, 0x00000000, 0x00000000, 0x00000000, 0x01470000,
    0x00003eb9, 0x00000000, 0x00000000, 0x00000000, 0x3f5300ad, 0x00000000, 0x00000000, 0x00000000,
    0x00480000, 0x00003fb8, 0x00000000, 0x00000000, 0x00000000, 0x3fef0011, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00004000, 0x00000000, 0xcdcd0000, 0xfdfdfdfd, 0xabababab, 0xabababab,
    0x00000000, 0x00000000, 0x5ff5f456, 0x000f5f58, 0x02cc6c78, 0x02cc0c28, 0xfeeefeee, 0xfeeefeee,
    0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee,
    0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee,
    0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee,
    0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee,
    0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee,
};

typedef struct _Framebuffer
{
    /* multi frame.*/
    gctUINT32  continuousNumber;

    /* For video stress test.*/
    gctUINT32 widthStep;
    gctUINT32 heightStep;
    gctUINT32 widthStepLength;
    gctUINT32 heightStepLength;

    gctUINT32  fb_phys[3];
    gctUINT32  fb_size;
    gctPOINTER fb_addr[3];
    gctUINT32  plane_size[3];
    gctUINT32  plane_num;

    gctBOOL   resolution_changed;
    gctBOOL   firstFrame;
    gctUINT32 width;
    gctUINT32 total_width;
    gctUINT32 height;
    gctUINT32 total_height;
    gctUINT32 stride[3];
    gctUINT32 stride_tile[3];
    gctUINT32 format;
    gctUINT32 bpp[3];
    gctUINT32 tiling;
    gctUINT32 yuv_type;

    gctUINT32 rotAngle;

    /* Allocated memory. */
    gctPOINTER  handle;
    gctPOINTER logical;
    gctUINT32  hardwareAddress;

    dc_framebuffer framebuffer;

    /* Source. */
    char       resource[100];

    /* Dither. */
    gctUINT32  dither;

    /* Gamma. */
    gctUINT32  gamma;

    char    tileStatusFileName[3][100];

    gctPOINTER    tileStatusHandle[2];
    gctUINT32    tileStatusHWAddress[2];
    gctPOINTER    tileStatusLogical[2];
}
Framebuffer;

typedef struct _Destination
{
    dc_dest     dest;

    /* Allocated memory. */
    gctPOINTER  handle;
    gctPOINTER logical;
    gctUINT32  hardwareAddress;

    /* Golden. */
    char       golden[100];

    gctPOINTER  goldenHandle;
    gctPOINTER goldenLogical;
    gctUINT32  goldenHardwareAddress;
}
Destination;

typedef struct _Cursor
{
    gctUINT32   cursor_type;
    gctUINT32   cursor_hsx;
    gctUINT32   cursor_hsy;
    gctUINT32   cursor_fg;
    gctUINT32   cursor_bg;
    gctUINT32   cursor_width;
    gctUINT32   cursor_height;
    gctUINT32   cursor_phys;
    gctPOINTER  cursor_addr;
    gctUINT32   cursor_size;
    gctUINT32   init_pos_x;
    gctUINT32   init_pos_y;
}
Cursor;

typedef struct _Overlay
{
    gctPOINTER plane_addr[3];
    gctUINT32  plane_size[3];
    gctUINT32  plane_num;

    dc_overlay overlay;

    /* Allocated memory. */
    gctPOINTER handle;
    gctPOINTER logical;
    gctUINT32  hardwareAddress;

    /* Source. */
    char       resource[100];

    gctUINT32  size;

    char    tileStatusFileName[3][100];

    gctPOINTER  tileStatusHandle[2];
    gctUINT32   tileStatusHWAddress[2];
    gctPOINTER  tileStatusLogical[2];

    /* For stress test.*/
    gctUINT32 step;
    gctUINT32 widthStepLength;
    gctUINT32 heightStepLength;
}
Overlay;

typedef struct _Gamma
{
#define GAMMA_TABLE_SIZE 256
    gctUINT16 gamma_table[GAMMA_TABLE_SIZE][3];
}
Gamma;

typedef struct _Config
{
    Framebuffer framebuffer;
    Overlay     overlay[DC_OVERLAY_NUM];
    Cursor      cursor;
    Gamma       gamma;
    Destination dest;

    gctUINT32 output_type;
    gctUINT32 output_format;
    gctUINT32 output_dbi_type;

    gctUINT32 memory;
    gctBOOL   mem2mem;

    gctBOOL use_global_setting;

/* Test group type.
    0. normal test.
    1. multi frame test.
    2. video stress test.
    3. overlay stress test.
*/
    viv_test_type test_type;
}
Config;

/* Alignment with a power of two value. */
#define gcmALIGN(n, align) \
(\
    ((n) + ((align) - 1)) & ~((align) - 1) \
)

/* FilterBlt information. */
#define gcvMAXKERNELSIZE        9
#define gcvSUBPIXELINDEXBITS    5

#define gcvSUBPIXELCOUNT \
    (1 << gcvSUBPIXELINDEXBITS)

#define gcvSUBPIXELLOADCOUNT \
    (gcvSUBPIXELCOUNT / 2 + 1)

#define gcvWEIGHTSTATECOUNT \
    (((gcvSUBPIXELLOADCOUNT * gcvMAXKERNELSIZE + 1) & ~1) / 2)

#define gcvKERNELTABLESIZE \
    (gcvSUBPIXELLOADCOUNT * gcvMAXKERNELSIZE * sizeof(gctUINT16))

#define gcvKERNELSTATES \
    (gcmALIGN(gcvKERNELTABLESIZE + 4, 8))

/* Filter types. */
typedef enum _vivFILTER_TYPE
{
    vivFILTER_SYNC = 0,
    vivFILTER_BLUR,
    vivFILTER_USER
}
vivFILTER_TYPE;

typedef struct _gcsFILTER_BLIT_ARRAY
{
    gctUINT8      kernelSize;
    gctUINT32     scaleFactor;
    gctUINT32_PTR kernelStates;
}
gcsFILTER_BLIT_ARRAY;

typedef gcsFILTER_BLIT_ARRAY * gcsFILTER_BLIT_ARRAY_PTR;

/******************************/
/*      General Function      */
/******************************/

gctINT viv_util_get_channel(
    gctUINT32 format,
    gctUINT16* a,
    gctUINT16* r,
    gctUINT16* g,
    gctUINT16* b
    );

gctINT viv_util_get_bpp(
    gctUINT32 format,
    gctUINT16* l1,
    gctUINT16* l2,
    gctUINT16* l3
    );

gctBOOL viv_util_is_yuv(
    gctUINT32 format
    );

gctUINT8* viv_util_format_string(
    gctUINT32 format
    );

gctUINT8* viv_util_output_string(
    gctUINT32 output
    );

gctINT viv_util_prepare_framebuffer_config(
    Framebuffer* framebuffer,
    gctUINT32 format,
    gctUINT32 tiling,
    gctUINT32 physical
    );

gctINT viv_util_free_framebuffer(
    Framebuffer *framebuffer
    );

gctINT viv_util_prepare_cursor_config(
    Cursor* cursor,
    gctUINT32 width,
    gctUINT32 height,
    gctUINT32 x,
    gctUINT32 y,
    gctUINT32 physical
    );

gctVOID viv_util_prepare_gamma_config(
    Gamma* gamma
    );

gctINT viv_util_clear_fb(
    Framebuffer* framebuffer,
    gctUINT32 color
    );

gctINT viv_util_clear_fb_rgb(
    Framebuffer* framebuffer,
    gctCHAR a,
    gctCHAR r,
    gctCHAR g,
    gctCHAR b
    );

gctVOID viv_util_clear_cursor(
    Cursor* cursor,
    gctUINT32 color
    );

gctINT viv_util_load_bmp2fb(
    Framebuffer* framebuffer,
    gctUINT8* filename
    );

gctINT viv_util_load_bmp_subarea(
    gctUINT8* file_name,
    gctUINT8* buf,
    gctUINT32 buf_width,
    gctUINT32 buf_height,
    gctUINT32 buf_stride,
    gctUINT8 *lut,
    gctUINT32 *ret_width,
    gctUINT32 *ret_height
    );

gctINT viv_util_load_raw2fb(
    Framebuffer* framebuffer,
    gctUINT8* filename
    );

gctINT viv_util_load_vimg(
    gctUINT8 *file,
    gctUINT32 format,
    gctUINT32 tile_mode,
    gctUINT32 compression,
    gctUINT8 **plane_addr,
    gctUINT32 *plane_width,
    gctUINT32 *plane_height,
    gctUINT32 *plane_stride,
    gctUINT32 plane_num,
    gctUINT8 **ts_buf,
    gctUINT32 *ret_buf_width,
    gctUINT32 *ret_buf_height,
    gctUINT32 *ret_img_width,
    gctUINT32 *ret_img_height
    );

gctINT viv_util_prepare_overlay_config(
    Overlay Overlay[],
    gctUINT32 Address
    );

gctINT viv_util_free_overlay(
    Overlay Overlay[]
    );

gctINT viv_util_load_bmp_area(
    gctUINT8* file_name,
    gctUINT8* buf,
    gctUINT32 buf_width,
    gctUINT32 buf_height,
    gctUINT32 buf_stride,
    gctUINT32 format,
    gctUINT32 tile_mode,
    gctUINT32 compression,
    gctUINT8 *lut,
    gctUINT32 *ret_buf_width,
    gctUINT32 *ret_buf_height,
    gctUINT32 *ret_img_width,
    gctUINT32 *ret_img_height
    );

gctINT viv_util_load_text_raw(
    gctUINT8* filename,
    gctUINT32* Buffer,
    gctUINT32 Size,
    gctUINT32 Format,
    gctUINT32 Height,
    gctUINT32 Stride[]
    );

vivSTATUS viv_util_get_tile_size(
    gctUINT32 format,
    gctUINT32 tile_mode,
    gctUINT32 compressed,
    gctUINT32 *tile_width_l1,
    gctUINT32 *tile_height_l1,
    gctUINT32 *tile_width_l2,
    gctUINT32 *tile_height_l2,
    gctUINT32 *tile_size_l2
    );

vivSTATUS viv_util_get_ts_bit(
    gctUINT32 format,
    gctUINT32 tile_mode,
    gctUINT32 alignment,
    gctUINT32 plane_num,
    gctUINT32 *tile_size,
    gctUINT32 *ts_bit_len
    );

vivSTATUS viv_util_load_raw_ts(
    gctUINT8 *file_name,
    gctUINT8 *buf,
    gctUINT32 ts_bit_len,
    gctUINT32 tile_width_l1,
    gctUINT32 tile_height_l1,
    gctUINT32 tile_width_l2,
    gctUINT32 tile_height_l2,
    gctUINT32 buf_width_in_tile_l1,
    gctUINT32 buf_height_in_tile_l1,
    gctUINT32 img_width_in_tile_l1,
    gctUINT32 img_height_in_tile_l1
    );

gctBOOL GalSaveDIB(
    gctPOINTER bits,
    gceSURF_FORMAT  format,
    gctUINT stride,
    gctUINT width,
    gctUINT height,
    const char *bmpFileName);

#define viv_print   viv_os_print
#define viv_sprint  viv_os_sprint

#endif

