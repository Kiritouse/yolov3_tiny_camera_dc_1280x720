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


#ifndef _VIV_DC_TYPE_H_
#define _VIV_DC_TYPE_H_

#include <linux/kernel.h>

/* Chip models. */
typedef enum _dcCHIPMODEL
{
    dc0000  = 0x0000,
    dc8000  = 0x0000,
    dculta_l  = 0x0000,
}
dcCHIPMODEL;

 /*
  * Chip feature.
  */
typedef enum _gceFEATURE
{
    gcvFEATURE_DC_SOURCE_ROTATION,
    gcvFEATURE_DC_TILED,
    gcvFEATURE_DC_YUV_L1,
    gcvFEATURE_DC_OVERLAY_SCALING,
    gcvFEATURE_DC_D30_OUTPUT,
    gcvFEATURE_DC_MMU,
    gcvFEATURE_DC_COMPRESSION,
    gcvFEATURE_DC_QOS,
    gcvFEATURE_DC_HORFILTER_5,
    gcvFEATURE_DC_10BIT,    /*Fix 10-bit NV12/ NV16 format cross 4K byte issue. */
    gcvFEATURE_DC_FIX_HWCURSOR,   /* Fix HW cursor issue, cursor can cross original framebuffer boundary. */
    /* Insert features above this comment only. */
    gcvFEATURE_COUNT    /* Not a feature. */
}
gceFEATURE;

 /*
    DPI display size.
 */
typedef enum _viv_display_size_type
{
    viv_display_640_480,
    viv_display_720_480,
    viv_display_1280_720,
    viv_display_1280_960,
    viv_display_1920_1080,
    viv_display_3840_2160,
    viv_display_1024_600_60,
}
viv_display_size_type;

 /*
  * Frame buffer format and output format type.
  */
typedef enum _viv_format_type {
    /* Unknown */
    vivUNKNOWN,

    /* General */
    vivARGB1555,
    vivARGB4444,
    vivRGB565,
    vivARGB8888,
    vivNV12,
    vivYUY2,

    /* DBI */
    vivD8R3G3B2,
    vivD8R4G4B4,
    vivD8R5G6B5,
    vivD8R6G6B6,
    vivD8R8G8B8,
    vivD9R6G6B6,
    vivD16R3G3B2,
    vivD16R4G4B4,
    vivD16R5G6B5,
    vivD16R6G6B6OP1,
    vivD16R6G6B6OP2,
    vivD16R8G8B8OP1,
    vivD16R8G8B8OP2,

    /* DPI */
    vivD16CFG1,
    vivD16CFG2,
    vivD16CFG3,
    vivD18CFG1,
    vivD18CFG2,
    vivD24    ,
}
viv_format_type;

 /*
  * Frame buffer mode.
  * Used in viv_conf_framebuffer_set_config()
  */
typedef enum _viv_tiling_type {
    vivLINEAR,
    vivTILED,
}
viv_tiling_type;

 /*
  * YUV type.
  * Used in viv_conf_yuv_set_config()
  */
typedef enum _viv_yuv_type {
    vivBT601,
    vivBT709,
}
viv_yuv_type;

 /*
  * Output type selection.
  * Used in viv_conf_output_select()
  */
typedef enum _viv_output_type {
    vivDBI,
    vivDPI,
}
viv_output_type;

 /*
  * DBI type.
  * Used in viv_conf_output_dbi_set_config()
  */
typedef enum _viv_dbi_type {
    vivDBI_AFIXED,
    vivDBI_ACLOCK,
    vivDBI_B,
}
viv_dbi_type;

 /*
  * DBI command flag
  * Used in viv_conf_output_dbi_set_command()
  */
typedef enum _viv_dbi_command_type {
    vivDBI_COMMAND_ADDRESS,
    vivDBI_COMMAND_MEM,
    vivDBI_COMMAND_DATA,
}
viv_dbi_command_type;

 /*
  * Cursor type.
  * Used in viv_conf_cursor_set_type()
  */
typedef enum _viv_cursor_type {
    vivCURSOR_ARGB,
    vivCURSOR_MASK,
}
viv_cursor_type;

/************************************************************************/
typedef enum _viv_status_type {
    vivSTATUS_HEAP_CORRUPTED = -7,
    vivSTATUS_OUT_OF_RESOURCES = -6,
    vivSTATUS_TIMEOUT = -5,
    vivSTATUS_NOT_SUPPORT = -4,
    vivSTATUS_OOM = -3,
    vivSTATUS_FAILED = -2,
    vivSTATUS_INVALID_ARGUMENTS = -1,

    vivSTATUS_OK = 0,
}
viv_status_type, vivSTATUS;

typedef enum _viv_file_type {
    vivFILE_BINARY = 0x001,

    vivFILE_READ   = 0x010,
    vivFILE_WRITE  = 0x100,

    vivFILE_READB  = vivFILE_READ  | vivFILE_BINARY,
    vivFILE_WRITEB = vivFILE_WRITE | vivFILE_BINARY,
}
viv_file_type;

typedef enum _viv_test_type {
    vivTEST_NORMAL          = 0x0,
    vivTEST_MULTIFRAME      = 0x1,
    vivTEST_VIDEOSTRESS     = 0x2,
    vivTEST_OVERLAYSTRESS   = 0x3,
}
viv_test_type;

typedef enum _viv_pool_type {
    gcvPOOL_CONTIGUOUS,
    gcvPOOL_DEFAULT,
}
gcvPOOL;

typedef int                     gctBOOL;
typedef int                     gctINT;
typedef unsigned int            gctUINT;
typedef unsigned char           gctUINT8;
typedef signed short            gctINT16;
typedef unsigned short          gctUINT16;
typedef gctUINT16 *                gctUINT16_PTR;
typedef signed int                gctINT32;
typedef unsigned int            gctUINT32;
typedef unsigned long long      gctUINT64;
typedef gctUINT32 *                gctUINT32_PTR;
typedef unsigned long           gctSIZE_T;
typedef char                    gctCHAR;
typedef float                   gctFLOAT;
typedef double                  gctDOUBLE;

typedef void                    gctVOID;
typedef void *                  gctPOINTER;

#define vivFALSE                0
#define vivTRUE                 1

#ifdef __cplusplus
#   define vivNULL              0
#else
#   define vivNULL              ((void *) 0)
#endif

#ifdef __cplusplus
#   define gcvNULL              0
#else
#   define gcvNULL              ((void *) 0)
#endif

typedef struct _viv_interface_query_chip_identity * viv_interface_query_chip_identity_ptr;
typedef struct _viv_interface_query_chip_identity {
    /* Chip model. */
    dcCHIPMODEL chipModel;
    /* Revision value.*/
    gctUINT32 chipRevision;
    /* Revision value.*/
    gctUINT32 chipPatchRevision;
    /* Chip date. */
    gctUINT32 chipInfo;
    /* Product ID */
    gctUINT32 productID;
    /* ECO ID. */
    gctUINT32 ecoID;
    /* Customer ID. */
    gctUINT32 customerID;
    /* Product Data */
    gctUINT32 productDate;
    /* Product time */
    gctUINT32 productTime;

}
viv_interface_query_chip_identity;


/************************************************************************/

/* For function enable/disable parameter */
#define SET_ENABLE                                  1
#define SET_DISABLE                                 0

/* For polarity parameter */
#define SET_POSITIVE                                1
#define SET_NEGATIVE                                0


#define vivMAX(a, b) ((a) > (b) ? (a) : (b))
#define vivMIN(a, b) ((a) < (b) ? (a) : (b))

#define vivPOW     pow
#define vivABS     abs
#define vivATOI    atoi
#define vivSIZEOF  sizeof

#define vivINFINITE ((gctUINT32)(~0U))

#define dcONERROR(func) \
    do \
    { \
        status = func; \
        if (status < 0) \
        { \
            goto OnError; \
        } \
    } \
    while (0)

#endif

