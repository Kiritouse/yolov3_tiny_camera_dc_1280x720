/*------------------------------------------------------------------------------
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

#ifndef __JPEGDECAPI_H__
#define __JPEGDECAPI_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*------------------------------------------------------------------------------

    Table of contents

    1. Include headers
    2. External compiler flags
    3. Module defines
    4. Local function prototypes
    5. Functions

------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
    1. Include headers
------------------------------------------------------------------------------*/
#include "basetype.h"
#include "decapicommon.h"
#include "dwl.h"

/*------------------------------------------------------------------------------
    2. Module defines
------------------------------------------------------------------------------*/
#define JPEGDEC_YCbCr400 0x080000U
#define JPEGDEC_YCbCr420_SEMIPLANAR 0x020001U
#define JPEGDEC_YCbCr422_SEMIPLANAR 0x010001U
#define JPEGDEC_YCbCr440 0x010004U
#define JPEGDEC_YCbCr411_SEMIPLANAR 0x100000U
#define JPEGDEC_YCbCr444_SEMIPLANAR 0x200000U

#define JPEGDEC_BASELINE 0x0
#define JPEGDEC_PROGRESSIVE 0x1
#define JPEGDEC_NONINTERLEAVED 0x2

/*------------------------------------------------------------------------------
    3. Data types
------------------------------------------------------------------------------*/

typedef void *JpegDecInst;

typedef enum {
  JPEGDEC_SLICE_READY = 2,
  JPEGDEC_FRAME_READY = 1,
  JPEGDEC_STRM_PROCESSED = 3,
  JPEGDEC_SCAN_PROCESSED = 4,
  JPEGDEC_END_OF_STREAM = 5,
  JPEGDEC_ABORTED = 7,
  JPEGDEC_WAITING_FOR_BUFFER = 9,
  JPEGDEC_OK = 0,
  JPEGDEC_ERROR = -1,
  JPEGDEC_UNSUPPORTED = -2,
  JPEGDEC_PARAM_ERROR = -3,
  JPEGDEC_MEMFAIL = -4,
  JPEGDEC_INITFAIL = -5,
  JPEGDEC_INVALID_STREAM_LENGTH = -6,
  JPEGDEC_STRM_ERROR = -7,
  JPEGDEC_INVALID_INPUT_BUFFER_SIZE = -8,
  JPEGDEC_HW_RESERVED = -9,
  JPEGDEC_INCREASE_INPUT_BUFFER = -10,
  JPEGDEC_SLICE_MODE_UNSUPPORTED = -11,
  JPEGDEC_NO_DECODING_BUFFER = -12,
  JPEGDEC_DWL_HW_TIMEOUT = -253,
  JPEGDEC_DWL_ERROR = -254,
  JPEGDEC_HW_BUS_ERROR = -255,
  JPEGDEC_SYSTEM_ERROR = -256,

  JPEGDEC_FORMAT_NOT_SUPPORTED = -1000
} JpegDecRet;

enum {
  JPEGDEC_NO_UNITS = 0,   /* No units, X and Y specify
                                 * the pixel aspect ratio    */
  JPEGDEC_DOTS_PER_INCH = 1,  /* X and Y are dots per inch */
  JPEGDEC_DOTS_PER_CM = 2 /* X and Y are dots per cm   */
};

enum {
  JPEGDEC_THUMBNAIL_JPEG = 0x10,
  JPEGDEC_THUMBNAIL_NOT_SUPPORTED_FORMAT = 0x11,
  JPEGDEC_NO_THUMBNAIL = 0x12
};

enum {
  JPEGDEC_IMAGE = 0,
  JPEGDEC_THUMBNAIL = 1
};

typedef void JpegDecMCStreamConsumed(u8 *stream, void *p_user_data);
typedef struct JpegDecMCConfig_ {
  u32 mc_enable;
  /*! Application provided callback for stream buffer processed. */
  JpegDecMCStreamConsumed *stream_consumed_callback;
} JpegDecMCConfig;

/* Image information */
typedef struct {
  u32 display_width;
  u32 display_height;
  u32 output_width;    /* Number of pixels/line in the image  */
  u32 output_height;   /* Number of lines in in the image     */
  u32 version;
  u32 units;
  u32 x_density;
  u32 y_density;
  u32 output_format;   /* JPEGDEC_YCbCr400
                             * JPEGDEC_YCbCr420
                             * JPEGDEC_YCbCr422
                             */
  u32 coding_mode; /* JPEGDEC_BASELINE
                         * JPEGDEC_PROGRESSIVE
                         * JPEGDEC_NONINTERLEAVED
                         */

  u32 thumbnail_type;  /* Thumbnail exist or not or not supported */
  u32 display_width_thumb;
  u32 display_height_thumb;
  u32 output_width_thumb;   /* Number of pixels/line in the image  */
  u32 output_height_thumb;  /* Number of lines in in the image     */
  u32 output_format_thumb;  /* JPEGDEC_YCbCr400
                                 * JPEGDEC_YCbCr420
                                 * JPEGDEC_YCbCr422
                                 */
  u32 coding_mode_thumb;    /* JPEGDEC_BASELINE
                                 * JPEGDEC_PROGRESSIVE
                                 * JPEGDEC_NONINTERLEAVED
                                 */
  DecPicAlignment align; /* alignment information, it is maybe default value defined inside ctrlSW,
                                               application uses it for output buffer allocation */

} JpegDecImageInfo;

/* Decoder input JFIF information */
typedef struct {
  struct DWLLinearMem stream_buffer;  /* input stream buffer */
  u32 stream_length;   /* input stream length */
  u32 buffer_size; /* input stream buffer size in input stream buffering mode */
  u32 dec_image_type;   /* Full image or Thumbnail to be decoded */
  u32 slice_mb_set; /* slice mode: mcu rows to decode */
  u32 ri_count;     /* count of restart intervals in input stream */
  u32 *ri_array;    /* offsets of beginnings of restart intervals */
  struct DWLLinearMem picture_buffer_y;    /* luma output address ==> if user allocated */
  struct DWLLinearMem picture_buffer_cb_cr; /* chroma output address ==> if user allocated */
  struct DWLLinearMem picture_buffer_cr; /* chroma output address ==> if user allocated */
  void *p_user_data;
  u8 *stream;  /* input stream bus address */
} JpegDecInput;

/* Decoder output */
typedef struct {
  struct JpegOutputInfo {
    struct DWLLinearMem output_picture_y;    /* Pointer to the Luma output image */
    struct DWLLinearMem output_picture_cb_cr; /* Pointer to the Chroma output image */
    struct DWLLinearMem output_picture_cr; /* Pointer to the Chroma output image */
    u32 output_width;           /* bytes in a pixel line (with padded bytes) */
    u32 output_height;
    u32 display_width;          /* valid pixels */
    u32 display_height;
    u32 output_width_thumb;
    u32 output_height_thumb;
    u32 display_width_thumb;
    u32 display_height_thumb;
    u32 pic_stride;
    u32 pic_stride_ch;
    enum DecPictureFormat output_format;
#ifdef SUPPORT_DEC400
    struct DWLLinearMem luma_table;           /**< Buffer properties *//*sunny add for tile status address*/
    struct DWLLinearMem chroma_table;
#endif
  } pictures[DEC_MAX_OUT_COUNT];
} JpegDecOutput;

typedef struct {
  u32 major;  /* API major version */
  u32 minor;  /* API minor version */

} JpegDecApiVersion;

typedef struct DecSwHwBuild  JpegDecBuild;

typedef struct {
  u32 next_buf_size;
  u32 buf_num;
  struct DWLLinearMem buf_to_free;
} JpegDecBufferInfo;

struct JpegDecConfig {
  DecPicAlignment align;
  PpUnitConfig ppu_config[DEC_MAX_PPU_COUNT];
  DelogoConfig delogo_params[2];
  u32 dec_image_type;
};
/*------------------------------------------------------------------------------
    4. Function prototypes
------------------------------------------------------------------------------*/

/* Version information */
JpegDecApiVersion JpegGetAPIVersion(void);

/* Build information */
JpegDecBuild JpegDecGetBuild(void);

/* Initialization */
JpegDecRet JpegDecInit(JpegDecInst * dec_inst,
                       const void *dwl,
		       enum DecDecoderMode decoder_mode,
                       JpegDecMCConfig *p_mcinit_cfg);

/* Release */
void JpegDecRelease(JpegDecInst dec_inst);

/* Get image information of the JFIF */
JpegDecRet JpegDecGetImageInfo(JpegDecInst dec_inst,
                               JpegDecInput * p_dec_in,
                               JpegDecImageInfo * p_image_info);

/* Decode JFIF */
JpegDecRet JpegDecDecode(JpegDecInst dec_inst,
                         JpegDecInput * p_dec_in, JpegDecOutput * p_dec_out);

/* Update HW in low latency mode */
void JpegDecUpdateStrmInfoCtrl(JpegDecInst dec_inst,
                               u32 last_flag,
                               u32 hw_stream_start_bus);


JpegDecRet JpegDecNextPicture(JpegDecInst dec_inst,
                              JpegDecOutput * output,
                              JpegDecImageInfo *info);

JpegDecRet JpegDecPictureConsumed(JpegDecInst dec_inst, JpegDecOutput * output);

JpegDecRet JpegDecEndOfStream(JpegDecInst dec_inst);

JpegDecRet JpegDecSetInfo(JpegDecInst dec_inst, struct JpegDecConfig *dec_cfg);

JpegDecRet JpegDecGetBufferInfo(JpegDecInst dec_inst, JpegDecBufferInfo *mem_info);

JpegDecRet JpegDecAddBuffer(JpegDecInst dec_inst, struct DWLLinearMem *info);
JpegDecRet JpegDecAbort(JpegDecInst dec_inst);

JpegDecRet JpegDecAbortAfter(JpegDecInst dec_inst);

/*------------------------------------------------------------------------------
    Prototype of the API trace funtion. Traces all API entries and returns.
    This must be implemented by the application using the decoder API!
    Argument:
        string - trace message, a null terminated string
------------------------------------------------------------------------------*/
void JpegDecTrace(const char *string);

#ifdef __cplusplus
}
#endif

#endif
