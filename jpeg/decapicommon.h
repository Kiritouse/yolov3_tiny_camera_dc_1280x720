/*--
          if (!ppu_cfg->monochrome && !mono_chrome)
--       Copyright (c) 2015-2017, VeriSilicon Inc. All rights reserved        --
--         Copyright (c) 2011-2014, Google Inc. All rights reserved.          --
--         Copyright (c) 2007-2010, Hantro OY. All rights reserved.           --
--                                                                            --
-- This software is confidential and proprietary and may be used only as      --
--   expressly authorized by VeriSilicon in a written licensing agreement.    --
--                                                                            --
--         This entire notice must be reproduced on all copies                --
--                       and may not be removed.                              --
--                                                                            --
--------------------------------------------------------------------------------
-- Redistribution and use in source and binary forms, with or without         --
-- modification, are permitted provided that the following conditions are met:--
--   * Redistributions of source code must retain the above copyright notice, --
--       this list of conditions and the following disclaimer.                --
--   * Redistributions in binary form must reproduce the above copyright      --
--       notice, this list of conditions and the following disclaimer in the  --
--       documentation and/or other materials provided with the distribution. --
--   * Neither the names of Google nor the names of its contributors may be   --
--       used to endorse or promote products derived from this software       --
--       without specific prior written permission.                           --
--------------------------------------------------------------------------------
-- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"--
-- AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE  --
-- IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE --
-- ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE  --
-- LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR        --
-- CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF       --
-- SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS   --
-- INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN    --
-- CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)    --
-- ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE --
-- POSSIBILITY OF SUCH DAMAGE.                                                --
--------------------------------------------------------------------------------
------------------------------------------------------------------------------*/

#ifndef DECAPICOMMON_H
#define DECAPICOMMON_H

#include "basetype.h"

/** Maximum number of cores supported in multi-core configuration */
/** For G2, multi-core currently not supported. */
#define MAX_ASIC_CORES 4

#define HEVC_NOT_SUPPORTED (u32)(0x00)
#define HEVC_MAIN_PROFILE (u32)(0x01)
#define HEVC_MAIN10_PROFILE (u32)(0x02)
#define HEVC_SUPPORTED (u32)(0x01)
#define VP9_NOT_SUPPORTED (u32)(0x00)
#define VP9_PROFILE0 (u32)(0x01)
#define VP9_PROFILE2_10BITS (u32)(0x02)
#define MPEG4_NOT_SUPPORTED (u32)(0x00)
#define MPEG4_SIMPLE_PROFILE (u32)(0x01)
#define MPEG4_ADVANCED_SIMPLE_PROFILE (u32)(0x02)
#define MPEG4_CUSTOM_NOT_SUPPORTED (u32)(0x00)
#define MPEG4_CUSTOM_FEATURE_1 (u32)(0x01)
#define H264_NOT_SUPPORTED (u32)(0x00)
#define H264_BASELINE_PROFILE (u32)(0x01)
#define H264_MAIN_PROFILE (u32)(0x02)
#define H264_HIGH_PROFILE (u32)(0x03)
#define VC1_NOT_SUPPORTED (u32)(0x00)
#define VC1_SIMPLE_PROFILE (u32)(0x01)
#define VC1_MAIN_PROFILE (u32)(0x02)
#define VC1_ADVANCED_PROFILE (u32)(0x03)
#define MPEG2_NOT_SUPPORTED (u32)(0x00)
#define MPEG2_MAIN_PROFILE (u32)(0x01)
#define JPEG_NOT_SUPPORTED (u32)(0x00)
#define JPEG_BASELINE (u32)(0x01)
#define JPEG_PROGRESSIVE (u32)(0x02)
#define PP_NOT_SUPPORTED (u32)(0x00)
#define PP_SUPPORTED (u32)(0x01)
#define PP_TILED_4X4 (u32)(0x20000000)
#define PP_DITHERING (u32)(0x10000000)
#define PP_SCALING (u32)(0x0C000000)
#define PP_DEINTERLACING (u32)(0x02000000)
#define PP_ALPHA_BLENDING (u32)(0x01000000)
#define PP_OUTP_ENDIAN (u32)(0x00040000)
#define PP_TILED_INPUT (u32)(0x0000C000)
#define PP_PIX_ACC_OUTPUT (u32)(0x40000000)
#define PP_ABLEND_CROP (u32)(0x80000000)
#define SORENSON_SPARK_NOT_SUPPORTED (u32)(0x00)
#define SORENSON_SPARK_SUPPORTED (u32)(0x01)
#define VP6_NOT_SUPPORTED (u32)(0x00)
#define VP6_SUPPORTED (u32)(0x01)
#define VP7_NOT_SUPPORTED (u32)(0x00)
#define VP7_SUPPORTED (u32)(0x01)
#define VP8_NOT_SUPPORTED (u32)(0x00)
#define VP8_SUPPORTED (u32)(0x01)
#define REF_BUF_NOT_SUPPORTED (u32)(0x00)
#define REF_BUF_SUPPORTED (u32)(0x01)
#define REF_BUF_INTERLACED (u32)(0x02)
#define REF_BUF_DOUBLE (u32)(0x04)
#define TILED_NOT_SUPPORTED (u32)(0x00)
#define TILED_8x4_SUPPORTED (u32)(0x01)
#define AVS_NOT_SUPPORTED (u32)(0x00)
#define AVS_SUPPORTED (u32)(0x01)
#define JPEG_EXT_NOT_SUPPORTED (u32)(0x00)
#define JPEG_EXT_SUPPORTED (u32)(0x01)
#define RV_NOT_SUPPORTED (u32)(0x00)
#define RV_SUPPORTED (u32)(0x01)
#define MVC_NOT_SUPPORTED (u32)(0x00)
#define MVC_SUPPORTED (u32)(0x01)
#define WEBP_NOT_SUPPORTED (u32)(0x00)
#define WEBP_SUPPORTED (u32)(0x01)
#define EC_NOT_SUPPORTED (u32)(0x00)
#define EC_SUPPORTED (u32)(0x01)
#define STRIDE_NOT_SUPPORTED (u32)(0x00)
#define STRIDE_SUPPORTED (u32)(0x01)
#define DOUBLE_BUFFER_NOT_SUPPORTED (u32)(0x00)
#define DOUBLE_BUFFER_SUPPORTED (u32)(0x01)
#define FIELD_DPB_NOT_SUPPORTED (u32)(0x00)
#define FIELD_DPB_SUPPORTED (u32)(0x01)
#define AVS_PLUS_NOT_SUPPORTED (u32)(0x00)
#define AVS_PLUS_SUPPORTED  (u32)(0x01)
#define ADDR64_ENV_NOT_SUPPORTED (u32)(0x00)
#define ADDR64_ENV_SUPPORTED (u32)(0x01)

#define H264_NOT_SUPPORTED_FUSE (u32)(0x00)
#define H264_FUSE_ENABLED (u32)(0x01)
#define MPEG4_NOT_SUPPORTED_FUSE (u32)(0x00)
#define MPEG4_FUSE_ENABLED (u32)(0x01)
#define MPEG2_NOT_SUPPORTED_FUSE (u32)(0x00)
#define MPEG2_FUSE_ENABLED (u32)(0x01)
#define SORENSON_SPARK_NOT_SUPPORTED_FUSE (u32)(0x00)
#define SORENSON_SPARK_ENABLED (u32)(0x01)
#define JPEG_NOT_SUPPORTED_FUSE (u32)(0x00)
#define JPEG_FUSE_ENABLED (u32)(0x01)
#define VP6_NOT_SUPPORTED_FUSE (u32)(0x00)
#define VP6_FUSE_ENABLED (u32)(0x01)
#define VP7_NOT_SUPPORTED_FUSE (u32)(0x00)
#define VP7_FUSE_ENABLED (u32)(0x01)
#define VP8_NOT_SUPPORTED_FUSE (u32)(0x00)
#define VP8_FUSE_ENABLED (u32)(0x01)
#define VC1_NOT_SUPPORTED_FUSE (u32)(0x00)
#define VC1_FUSE_ENABLED (u32)(0x01)
#define JPEG_PROGRESSIVE_NOT_SUPPORTED_FUSE (u32)(0x00)
#define JPEG_PROGRESSIVE_FUSE_ENABLED (u32)(0x01)
#define REF_BUF_NOT_SUPPORTED_FUSE (u32)(0x00)
#define REF_BUF_FUSE_ENABLED (u32)(0x01)
#define AVS_NOT_SUPPORTED_FUSE (u32)(0x00)
#define AVS_FUSE_ENABLED (u32)(0x01)
#define RV_NOT_SUPPORTED_FUSE (u32)(0x00)
#define RV_FUSE_ENABLED (u32)(0x01)
#define MVC_NOT_SUPPORTED_FUSE (u32)(0x00)
#define MVC_FUSE_ENABLED (u32)(0x01)

#define PP_NOT_SUPPORTED_FUSE (u32)(0x00)
#define PP_FUSE_ENABLED (u32)(0x01)
#define PP_FUSE_DEINTERLACING_ENABLED (u32)(0x40000000)
#define PP_FUSE_ALPHA_BLENDING_ENABLED (u32)(0x20000000)
#define MAX_PP_OUT_WIDHT_1920_FUSE_ENABLED (u32)(0x00008000)
#define MAX_PP_OUT_WIDHT_1280_FUSE_ENABLED (u32)(0x00004000)
#define MAX_PP_OUT_WIDHT_720_FUSE_ENABLED (u32)(0x00002000)
#define MAX_PP_OUT_WIDHT_352_FUSE_ENABLED (u32)(0x00001000)

#define BT601 0
#define BT601_L 1
#define BT709 2
#define BT709_L 3
#define BT2020 4
#define BT2020_L 5

#define VSI_LINEAR 0
#define LANCZOS 1
#define NEAREST  2
#define BI_LINEAR 3
#define BICUBIC 4
#define SPLINE 5
#define BOX 6
#define FAST_LINEAR 7

#define TILED4x4 0
#define TILED8x8 1
#define TILED16x16 2

#define DOWN_ROUND 0
#define NO_ROUND   1
#define UP_ROUND   2
/* Maximum output channels from xxxDecNexPicture() */
#define DEC_MAX_OUT_COUNT 6
#define DEC_MAX_PPU_COUNT 6
/* Picture dimensions are checked
   currently in vp9 and hecv api code */
#if !defined(MODEL_SIMULATION) || defined(HW_PIC_DIMENSIONS)
#define MIN_PIC_WIDTH 144
#define MIN_PIC_HEIGHT 144

#define MIN_PIC_WIDTH_VP9 72
#define MIN_PIC_HEIGHT_VP9 72

/* DTRC minimum size = 96x8, So the minimum size for vp9 decoding
   should be redefined if DTRC is enabled */
#define MIN_PIC_WIDTH_VP9_EN_DTRC 96
#define MIN_PIC_HEIGHT_VP9_EN_DTRC 72

#else /* MODEL_SIMULATION */
#define MIN_PIC_WIDTH 8
#define MIN_PIC_HEIGHT 8

#define MIN_PIC_WIDTH_VP9 8
#define MIN_PIC_HEIGHT_VP9 8

#define MIN_PIC_WIDTH_VP9_EN_DTRC 8
#define MIN_PIC_HEIGHT_VP9_EN_DTRC 8

#endif /* MODEL_SIMULATION */

struct DecHwConfig {
  u32 mpeg4_support;        /* one of the MPEG4 values defined above */
  u32 custom_mpeg4_support; /* one of the MPEG4 custom values defined above */
  u32 h264_support;         /* one of the H264 values defined above */
  u32 vc1_support;          /* one of the VC1 values defined above */
  u32 mpeg2_support;        /* one of the MPEG2 values defined above */
  u32 jpeg_support;         /* one of the JPEG values defined above */
  u32 jpeg_prog_support;  /* one of the Progressive JPEG values defined above */
  u32 max_dec_pic_width;  /* maximum picture width in decoder */
  u32 max_dec_pic_height; /* maximum picture height in decoder */
  u32 pp_support;         /* PP_SUPPORTED or PP_NOT_SUPPORTED */
  u32 pp_config;          /* Bitwise list of PP function */
  u32 max_pp_out_pic_width;   /* maximum post-processor output picture width */
  u32 sorenson_spark_support; /* one of the SORENSON_SPARK values defined above
                                 */
  u32 ref_buf_support;       /* one of the REF_BUF values defined above */
  u32 tiled_mode_support;    /* one of the TILED values defined above */
  u32 vp6_support;           /* one of the VP6 values defined above */
  u32 vp7_support;           /* one of the VP7 values defined above */
  u32 vp8_support;           /* one of the VP8 values defined above */
  u32 vp9_support;           /* HW supports VP9 */
  u32 avs_support;           /* one of the AVS values defined above */
  u32 jpeg_esupport;         /* one of the JPEG EXT values defined above */
  u32 rv_support;            /* one of the HUKKA values defined above */
  u32 mvc_support;           /* one of the MVC values defined above */
  u32 webp_support;          /* one of the WEBP values defined above */
  u32 ec_support;            /* one of the EC values defined above */
  u32 stride_support;        /* HW supports separate Y and C strides */
  u32 field_dpb_support;     /* HW supports field-mode DPB */
  u32 avs_plus_support;      /* one of the AVS PLUS values defined above */
  u32 addr64_support;         /* HW supports 64bit addressing */
  u32 hevc_support;          /* HW supports HEVC */
  u32 av1_support;           /* HW supports AV1 */
  u32 double_buffer_support; /* Decoder internal reference double buffering */

  u32 hevc_main10_support;  /* HW supports HEVC Main10 profile*/
  u32 vp9_10bit_support;     /* HW supports VP9 10 bits profile */
  u32 ds_support;            /* HW supports down scaling. */
  u32 rfc_support;           /* HW supports reference frame compression. */
  u32 ring_buffer_support;   /* HW supports ring buffer. */

  u32 fmt_p010_support;      /* HW supports P010 format. */
  u32 fmt_customer1_support; /* HW supports special customized format */
  u32 mrb_prefetch;          /* HW supports Multi-Reference Blocks Prefetch */
};

struct DecSwHwBuild {
  u32 sw_build;                 /* Software build ID */
  u32 hw_build;                 /* Hardware build ID */
  struct DecHwConfig hw_config[MAX_ASIC_CORES]; /* Hardware configuration */
};

/**
 * \enum DecDpbFlags
 * \brief DPB flags to control reference picture format etc.
 * \ingroup common_group
 */
enum DecDpbFlags {
  /* Reference frame formats */
  DEC_REF_FRM_RASTER_SCAN = 0x0,
  DEC_REF_FRM_TILED_DEFAULT = 0x1,

  /* Flag to allow SW to use DPB field ordering on interlaced content */
  DEC_DPB_ALLOW_FIELD_ORDERING = 0x40000000
};

#define DEC_REF_FRM_FMT_MASK 0x01
#define LOW_LATENCY_PACKET_SIZE 256

/**
 * \enum DecDpbMode
 * \brief Modes for storing content into DPB.
 * \ingroup common_group
 */
enum DecDpbMode {
  DEC_DPB_FRAME = 0,
  DEC_DPB_INTERLACED_FIELD = 1
};

/**
 * \enum DecDecoderMode
 * \brief Decoder working mode.
 * \ingroup common_group
 */
enum DecDecoderMode {
  DEC_NORMAL =           0x00000000,
  DEC_LOW_LATENCY =      0x00000001,
  DEC_SECURITY =         0x00000002,
  DEC_PARTIAL_DECODING = 0x00000004,
  DEC_INTRA_ONLY =       0x00000008
};

enum DelogoMode {
  PIXEL_NO_DELOGO = 0,
  PIXEL_REPLACE = 1,
  PIXEL_INTERPOLATION = 2
};
/* DEPRECATED!!! do not use in new applications! */
#define DEC_DPB_DEFAULT DEC_DPB_FRAME

/**
 * \enum DecPictureFormat
 * \brief Output picture format types.
 * \ingroup common_group
 */
enum DecPictureFormat {
  DEC_OUT_FRM_TILED_4X4 = 0,
  DEC_OUT_FRM_TILED_8X4 = 1,
  DEC_OUT_FRM_RASTER_SCAN = 2, /**< a.k.a. SEMIPLANAR_420 */
  DEC_OUT_FRM_PLANAR_420 = 3,
  DEC_OUT_FRM_TILED_8X8 = 4,
  DEC_OUT_FRM_TILED_16X16 = 5,
  DEC_OUT_FRM_MONOCHROME,       /**< a.k.a. YUV400 */
  DEC_OUT_FRM_RFC,
  DEC_OUT_FRM_RGB,
  DEC_OUT_FRM_DEC400,
  /* YUV420 */
  DEC_OUT_FRM_YUV420TILE,        /**< YUV420, 8-bit, Tile4x4 */
  DEC_OUT_FRM_YUV420TILE_PACKED, /**< Reference frame format */
  DEC_OUT_FRM_YUV420TILE_P010,
  DEC_OUT_FRM_YUV420TILE_1010,
  DEC_OUT_FRM_YUV420SP,          /**< YUV420, 8-bit, semi-planar */
  DEC_OUT_FRM_YUV420SP_PACKED,
  DEC_OUT_FRM_YUV420SP_P010,
  DEC_OUT_FRM_YUV420SP_1010,
  DEC_OUT_FRM_YUV420P,            /**< YUV420, 8-bit, planar */
  DEC_OUT_FRM_YUV420P_PACKED,
  DEC_OUT_FRM_YUV420P_P010,
  DEC_OUT_FRM_YUV420P_1010,
  DEC_OUT_FRM_YUV420P_I010,
   /* YUV400 */
  DEC_OUT_FRM_YUV400TILE,        /**< YUV420, 8-bit, Tile4x4 */
  DEC_OUT_FRM_YUV400TILE_P010,
  DEC_OUT_FRM_YUV400TILE_1010,
  DEC_OUT_FRM_YUV400,            /**< YUV420, 8-bit, planar */
  DEC_OUT_FRM_YUV400_P010,
  DEC_OUT_FRM_YUV400_1010,
  /* NV21 */
  DEC_OUT_FRM_NV21TILE,        /**< YUV420, 8-bit, Tile4x4 */
  DEC_OUT_FRM_NV21TILE_PACKED, /**< Reference frame format */
  DEC_OUT_FRM_NV21TILE_P010,
  DEC_OUT_FRM_NV21TILE_1010,
  DEC_OUT_FRM_NV21SP,          /**< YUV420, 8-bit, semi-planar */
  DEC_OUT_FRM_NV21SP_PACKED,
  DEC_OUT_FRM_NV21SP_P010,
  DEC_OUT_FRM_NV21SP_1010,
  DEC_OUT_FRM_NV21P,            /**< YUV420, 8-bit, planar */
  DEC_OUT_FRM_NV21P_PACKED,
  DEC_OUT_FRM_NV21P_P010,
  DEC_OUT_FRM_NV21P_1010,
  /* RGB */
  DEC_OUT_FRM_RGB888,
  DEC_OUT_FRM_BGR888,
  DEC_OUT_FRM_R16G16B16,
  DEC_OUT_FRM_B16G16R16,
  DEC_OUT_FRM_RGB888_P,
  DEC_OUT_FRM_BGR888_P,
  DEC_OUT_FRM_R16G16B16_P,
  DEC_OUT_FRM_B16G16R16_P,
  DEC_OUT_FRM_ARGB888,
  DEC_OUT_FRM_ABGR888,
  DEC_OUT_FRM_A2R10G10B10,
  DEC_OUT_FRM_A2B10G10R10,
  DEC_OUT_FRM_XRGB888,
  DEC_OUT_FRM_XBGR888
};

#define IS_PIC_PACKED_RGB(fmt) ((fmt) == DEC_OUT_FRM_RGB888 || \
  (fmt) == DEC_OUT_FRM_BGR888 || \
  (fmt) == DEC_OUT_FRM_R16G16B16 || \
  (fmt) == DEC_OUT_FRM_B16G16R16 || \
  (fmt) == DEC_OUT_FRM_ARGB888 || \
  (fmt) == DEC_OUT_FRM_ABGR888 || \
  (fmt) == DEC_OUT_FRM_A2R10G10B10 || \
  (fmt) == DEC_OUT_FRM_A2B10G10R10 || \
  (fmt) == DEC_OUT_FRM_XRGB888 || \
  (fmt) == DEC_OUT_FRM_XBGR888)

#define IS_PIC_PLANAR_RGB(fmt) ((fmt) == DEC_OUT_FRM_RGB888_P || \
  (fmt) == DEC_OUT_FRM_BGR888_P || \
  (fmt) == DEC_OUT_FRM_R16G16B16_P || \
  (fmt) == DEC_OUT_FRM_B16G16R16_P)

#define IS_PIC_MONOCHROME(fmt) ((fmt) == DEC_OUT_FRM_MONOCHROME || \
  (fmt) == DEC_OUT_FRM_YUV400TILE || \
  (fmt) == DEC_OUT_FRM_YUV400TILE_P010 || \
  (fmt) == DEC_OUT_FRM_YUV400 || \
  (fmt) == DEC_OUT_FRM_YUV400_1010 || \
  (fmt) == DEC_OUT_FRM_YUV400_P010)

#define IS_PIC_PLANAR(fmt) ((fmt) == DEC_OUT_FRM_PLANAR_420 || \
  (fmt) == DEC_OUT_FRM_YUV420P || \
  (fmt) == DEC_OUT_FRM_YUV420P_PACKED || \
  (fmt) == DEC_OUT_FRM_YUV420P_P010 || \
  (fmt) == DEC_OUT_FRM_YUV420P_I010 || \
  (fmt) == DEC_OUT_FRM_NV21P || \
  (fmt) == DEC_OUT_FRM_NV21P_PACKED || \
  (fmt) == DEC_OUT_FRM_NV21P_P010 || \
  IS_PIC_PLANAR_RGB(fmt))

#define IS_PIC_NV21(fmt) ((fmt) == DEC_OUT_FRM_NV21TILE || \
  (fmt) == DEC_OUT_FRM_NV21TILE_PACKED || \
  (fmt) == DEC_OUT_FRM_NV21TILE_P010 || \
  (fmt) == DEC_OUT_FRM_NV21SP || \
  (fmt) == DEC_OUT_FRM_NV21SP_PACKED || \
  (fmt) == DEC_OUT_FRM_NV21SP_P010 || \
  (fmt) == DEC_OUT_FRM_NV21P || \
  (fmt) == DEC_OUT_FRM_NV21P_PACKED || \
  (fmt) == DEC_OUT_FRM_NV21P_P010 )

#define IS_PIC_SEMIPLANAR(fmt) ((fmt) == DEC_OUT_FRM_YUV420SP || \
  (fmt) == DEC_OUT_FRM_YUV420SP_PACKED || \
  (fmt) == DEC_OUT_FRM_YUV420SP_P010 || \
  (fmt) == DEC_OUT_FRM_NV21SP || \
  (fmt) == DEC_OUT_FRM_NV21SP_PACKED || \
  (fmt) == DEC_OUT_FRM_NV21SP_P010 || \
  (fmt) == DEC_OUT_FRM_RASTER_SCAN)

#define IS_PIC_8BIT(fmt) ((fmt) == DEC_OUT_FRM_YUV420TILE || \
  (fmt) == DEC_OUT_FRM_YUV420SP || \
  (fmt) == DEC_OUT_FRM_YUV420P || \
  (fmt) == DEC_OUT_FRM_YUV400TILE || \
  (fmt) == DEC_OUT_FRM_YUV400 || \
  (fmt) == DEC_OUT_FRM_NV21TILE || \
  (fmt) == DEC_OUT_FRM_NV21SP || \
  (fmt) == DEC_OUT_FRM_NV21P || \
  (fmt) == DEC_OUT_FRM_RGB888_P || \
  (fmt) == DEC_OUT_FRM_BGR888_P || \
  (fmt) == DEC_OUT_FRM_RGB888 || \
  (fmt) == DEC_OUT_FRM_BGR888 || \
  (fmt) == DEC_OUT_FRM_ARGB888 || \
  (fmt) == DEC_OUT_FRM_ABGR888 || \
  (fmt) == DEC_OUT_FRM_XRGB888 || \
  (fmt) == DEC_OUT_FRM_XBGR888)

#define IS_PIC_10BIT(fmt) ((fmt) == DEC_OUT_FRM_YUV420TILE_1010 || \
  (fmt) == DEC_OUT_FRM_YUV420SP_1010 || \
  (fmt) == DEC_OUT_FRM_YUV420P_1010 || \
  (fmt) == DEC_OUT_FRM_YUV400TILE_1010 || \
  (fmt) == DEC_OUT_FRM_YUV400_1010 || \
  (fmt) == DEC_OUT_FRM_NV21TILE_1010 || \
  (fmt) == DEC_OUT_FRM_NV21SP_1010 || \
  (fmt) == DEC_OUT_FRM_NV21P_1010)

#define IS_PIC_16BIT(fmt) ((fmt) == DEC_OUT_FRM_YUV420TILE_P010 || \
  (fmt) == DEC_OUT_FRM_YUV420SP_P010 || \
  (fmt) == DEC_OUT_FRM_YUV420P_P010 || \
  (fmt) == DEC_OUT_FRM_YUV420P_I010 || \
  (fmt) == DEC_OUT_FRM_YUV400TILE_P010 || \
  (fmt) == DEC_OUT_FRM_YUV400_P010 || \
  (fmt) == DEC_OUT_FRM_NV21TILE_P010 || \
  (fmt) == DEC_OUT_FRM_NV21SP_P010 || \
  (fmt) == DEC_OUT_FRM_NV21P_P010 || \
  (fmt) == DEC_OUT_FRM_R16G16B16_P || \
  (fmt) == DEC_OUT_FRM_B16G16R16_P || \
  (fmt) == DEC_OUT_FRM_R16G16B16 || \
  (fmt) == DEC_OUT_FRM_B16G16R16)

#define IS_PIC_24BIT(fmt) ((fmt) == DEC_OUT_FRM_RGB888 || \
  (fmt) == DEC_OUT_FRM_BGR888)

#define IS_PIC_32BIT(fmt) ((fmt) == DEC_OUT_FRM_ARGB888 || \
                           (fmt) == DEC_OUT_FRM_ABGR888 || \
                           (fmt) == DEC_OUT_FRM_A2R10G10B10 || \
                           (fmt) == DEC_OUT_FRM_A2B10G10R10 || \
                           (fmt) == DEC_OUT_FRM_XRGB888 || \
                           (fmt) == DEC_OUT_FRM_XBGR888)

#define IS_PIC_48BIT(fmt) ((fmt) == DEC_OUT_FRM_R16G16B16 || \
  (fmt) == DEC_OUT_FRM_B16G16R16)

#define IS_PIC_TILE(fmt) ((fmt) == DEC_OUT_FRM_TILED_4X4 || \
  (fmt) == DEC_OUT_FRM_TILED_8X4 || \
  (fmt) == DEC_OUT_FRM_YUV420TILE || \
  (fmt) == DEC_OUT_FRM_YUV420TILE_PACKED || \
  (fmt) == DEC_OUT_FRM_YUV420TILE_P010 || \
  (fmt) == DEC_OUT_FRM_YUV420TILE_1010 || \
  (fmt) == DEC_OUT_FRM_YUV400TILE || \
  (fmt) == DEC_OUT_FRM_YUV400TILE_P010 || \
  (fmt) == DEC_OUT_FRM_YUV400TILE_1010 || \
  (fmt) == DEC_OUT_FRM_NV21TILE || \
  (fmt) == DEC_OUT_FRM_NV21TILE_PACKED || \
  (fmt) == DEC_OUT_FRM_NV21TILE_P010 || \
  (fmt) == DEC_OUT_FRM_NV21TILE_1010)

#define IS_PIC_RFC(fmt) ((fmt) == DEC_OUT_FRM_RFC)

#define IS_PIC_DEC400(fmt) ((fmt) == DEC_OUT_FRM_DEC400)

#define IS_PIC_TILED8x8(fmt) ((fmt) == DEC_OUT_FRM_TILED_8X8)

#define IS_PIC_TILED16x16(fmt) ((fmt) == DEC_OUT_FRM_TILED_16X16)
/** Picture coding type */
enum DecPicCodingType {
  DEC_PIC_TYPE_I           = 0,
  DEC_PIC_TYPE_P           = 1,
  DEC_PIC_TYPE_B           = 2,
  DEC_PIC_TYPE_D           = 3,
  DEC_PIC_TYPE_FI          = 4,
  DEC_PIC_TYPE_BI          = 5
};

/**
 * \enum DecPicturePixelFormat
 * \brief Output picture pixel format types for raster scan or down scale output.
 * \ingroup common_group
 */
enum DecPicturePixelFormat {
  DEC_OUT_PIXEL_DEFAULT = 0,    /**< packed pixel: each pixel in at most 10 bits as reference buffer */
  DEC_OUT_PIXEL_P010 = 1,       /**< a.k.a. MS P010 format */
  DEC_OUT_PIXEL_CUSTOMER1 = 2,  /**< customer format: a 128-bit burst output in packed little endian format */
  DEC_OUT_PIXEL_CUT_8BIT = 3,   /**< cut 10 bit to 8 bit per pixel */
  DEC_OUT_PIXEL_RFC = 4,        /**< compressed tiled output */
  DEC_OUT_PIXEL_1010 = 5,
};

/* error handling */
enum DecErrorHandling {
  /* Data property */
  DEC_EC_PIC_COPY_REF = 0x1,       /* Copy whole data from reference picture buffer to current picture buffer. */
  DEC_EC_PIC_PARTIAL = 0x2,        /* Copy partial picture data from reference picture buffer to current picture buffer. */
  DEC_EC_PIC_PARTIAL_IGNORE = 0x4, /* Ignore partial invalid data in current picture buffer */
  DEC_EC_PIC_ALL_IGNORE = 0x8,     /* Do nothing for the data in current picture buffer */
  /* Reference property */
  DEC_EC_REF_REPLACE = 0x100,      /* If one reference is erroneous, relpace it with nearest correct ref picture in POC distance. */
  DEC_EC_REF_NEXT_IDR = 0x200,     /* If one reference is erroneous, mark current picture as erroneous directly until a new IDR picture encountered. */
  DEC_EC_REF_NEXT_I = 0x400,       /* If one reference is erroneous, mark current picture as erroneous directly until a new I picture encountered. */
  /* Output property */
  DEC_EC_OUT_ALL = 0x10000,            /* Output all pictures including the picture marked as erroneous picture. */
  DEC_EC_OUT_NO_ERROR = 0x20000,       /* Output correct pictures, other pictures are discarded. */
  DEC_EC_OUT_FIRST_FIELD_OK = 0x40000, /* Output correct pictures and the pictures contain correct 1st field, other pictures are discarded. */
  /* Typical combinations */
  DEC_EC_PICTURE_FREEZE = DEC_EC_PIC_COPY_REF,                       /* If current picture is erroneous, freeze current picture */
  DEC_EC_VIDEO_FREEZE = (DEC_EC_PIC_COPY_REF | DEC_EC_REF_NEXT_IDR), /* If current picture is erroneous, freeze whole picture until a new IDR picture encountered. */
  DEC_EC_PARTIAL_FREEZE = DEC_EC_PIC_PARTIAL,                        /* If current picture is erroneous in partial space , freeze partial picture */
  DEC_EC_PARTIAL_IGNORE = DEC_EC_PIC_PARTIAL_IGNORE,                 /* If current picture is erroneous in partial space, remove the erroneous flag and treat it as correct picture */
  DEC_EC_FAST_FREEZE = (DEC_EC_PIC_ALL_IGNORE | DEC_EC_REF_NEXT_I | DEC_EC_OUT_NO_ERROR)  /* If current picture is erroneous, mark all pictures as erroneous until a new I picture encountered, and discard erroneous pictures */
};

struct DecCropCfg {
  u32 crop_x;
  u32 crop_y;
  u32 crop_w;
  u32 crop_h;
  u32 crop_enabled;
};

enum SCALE_MODE {
  NON_SCALE,
  FIXED_DOWNSCALE,
  FLEXIBLE_SCALE
} ;

struct DecFixedScaleCfg {
  u32 down_scale_x;
  u32 down_scale_y;
  u32 fixed_scale_enabled;
};

/**
 * \enum DecPicAlignment
 * \brief Stride alignment: aligned to 8/16/.../512 bytes.
 * \ingroup common_group
 */
typedef enum {
  DEC_ALIGN_1B = 0,
  DEC_ALIGN_8B = 3,
  DEC_ALIGN_16B,
  DEC_ALIGN_32B,
  DEC_ALIGN_64B,
  DEC_ALIGN_128B,
  DEC_ALIGN_256B,
  DEC_ALIGN_512B,
  DEC_ALIGN_1024B,
  DEC_ALIGN_2048B,
} DecPicAlignment;

typedef struct _DelogoConfig {
  u32 enabled;
  u32 x;
  u32 y;
  u32 w;
  u32 h;
  u32 show;
  enum DelogoMode mode;
  u32 Y;
  u32 U;
  u32 V;
} DelogoConfig;

typedef struct _PpUnitConfig {
  u32 enabled;    /* PP unit enabled */
  u32 tiled_e;    /* PP unit tiled4x4 output enabled */
  u32 rgb;        /* RGB output enabled */
  u32 rgb_planar; /* RGB output planar output enabled */
  u32 cr_first;   /* CrCb instead of CbCr */
  u32 shaper_enabled;
  u32 shaper_no_pad;
  u32 dec400_enabled; /* sw control shaper and dec400  */
  u32 planar;     /* Planar output */
  DecPicAlignment align;  /* pp output alignment */
  /* Stride for Y/C plane. SW should use the stride calculated from SW if it's
     set to 0. When not 0, SW should check the validation of the value. */
  u32 ystride;
  u32 cstride;
  struct {
    u32 enabled;  /* whether cropping is enabled */
    u32 set_by_user;   /* cropping set by user, use this variable to record
                        * whether user set crop.*/
    u32 x;        /* cropping start x */
    u32 y;        /* cropping start y */
    u32 width;    /* cropping width */
    u32 height;   /* cropping height */
  } crop;
  struct {
    u32 enabled;
    u32 x;        /* cropping start x */
    u32 y;        /* cropping start y */
    u32 width;    /* cropping width */
    u32 height;   /* cropping height */
  } crop2;
  struct {
    u32 enabled;  /* whether scaling is enabled */
    u32 set_by_user;   /* scaling set by user, use this variable to record
                        * whether user set scale.*/
    u32 ratio_x;  /* 0 indicate flexiable mode, or 1/2/4/8 indicate ratio */
    u32 ratio_y;
    u32 width;    /* scaled output width */
    u32 height;   /* scaled output height */
  } scale;
  u32 monochrome; /* PP output monochrome (luma only) for YUV output */
  u32 out_p010;
  u32 out_1010;
  u32 out_I010;
  u32 out_L010;
  u32 out_be;
  u32 out_cut_8bits;
  u32 video_range;  /* 1 - full range, 0 - limited range */
  u32 range_max;
  u32 range_min;
  u32 out_format;
  u32 rgb_format;   /* RGB output format: RGB888/BGR888/R16G16B16/... */
  u32 rgb_stan;     /* color conversion standard applied to set coeffs */
  u32 rgb_alpha;
  u32 pp_filter;
  u32 x_filter_param;
  u32 y_filter_param;
  u32 afbc_mode;
  u32 src_sel_mode;
  u32 pad_sel;
  u32 pad_Y;
  u32 pad_U;
  u32 pad_V;
} PpUnitConfig;
#endif /* DECAPICOMMON_H */
