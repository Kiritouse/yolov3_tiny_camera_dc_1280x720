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

#ifndef TB_DEFS_H
#define TB_DEFS_H

#ifdef _ASSERT_USED
#include <assert.h>
#endif

#include <stdio.h>

#include "basetype.h"
#include "decapicommon.h"
/*------------------------------------------------------------------------------
    Generic data type stuff
------------------------------------------------------------------------------*/

typedef enum {
  TB_FALSE = 0,
  TB_TRUE = 1
} TBBool;

/*------------------------------------------------------------------------------
    Test bench configuration    u32 stride_enable;

------------------------------------------------------------------------------*/
struct TBParams {
  char packet_by_packet[9];
  char nal_unit_stream[9];
  u32 seed_rnd;
  char stream_bit_swap[24];
  char stream_bit_loss[24];
  char stream_packet_loss[24];
  char stream_header_corrupt[9];
  char stream_truncate[9];
  char slice_ud_in_packet[9];
  u32 first_trace_frame;

  u32 extra_cu_ctrl_eof;
  u32 memory_page_size;
  i32 ref_frm_buffer_size;

  u32 unified_reg_fmt;
};

struct TBDecParams {
  char output_picture_endian[14];
  u32 bus_burst_length;
  u32 asic_service_priority;
  char output_format[12];
  u32 latency_compensation;
  char clock_gating[9];
  u32 clk_gate_decoder;
  u32 clk_gate_decoder_idle;
  char data_discard[9];

  char memory_allocation[9];
  char rlc_mode_forced[9];
  char error_concealment[15];

  u32 jpeg_mcus_slice;
  u32 jpeg_input_buffer_size;

  u32 refbu_enable;
  u32 refbu_disable_interlaced;
  u32 refbu_disable_double;
  u32 refbu_disable_eval_mode;
  u32 refbu_disable_checkpoint;
  u32 refbu_disable_offset;
  u32 refbu_data_excess_max_pct;
  u32 refbu_disable_top_bot_sum;

  u32 mpeg2_support;
  u32 vc1_support;
  u32 jpeg_support;
  u32 mpeg4_support;
  u32 custom_mpeg4_support;
  u32 h264_support;
  u32 vp6_support;
  u32 vp7_support;
  u32 vp8_support;
  u32 prog_jpeg_support;
  u32 sorenson_support;
  u32 avs_support;
  u32 rv_support;
  u32 mvc_support;
  u32 webp_support;
  u32 ec_support;
  u32 max_dec_pic_width;
  u32 max_dec_pic_height;
  u32 hw_version;
  u32 cache_version;
  u32 hw_build;
  u32 hw_build_id;
  u32 bus_width;
  u32 bus_width64bit_enable;
  u32 latency;
  u32 non_seq_clk;
  u32 seq_clk;
  u32 support_non_compliant;
  u32 jpeg_esupport;
  u32 hevc_main10_support;
  u32 vp9_profile2_support;
  u32 rfc_support;
  u32 ds_support;
  u32 ring_buffer_support;
  u32 mrb_prefetch;
  u32 format_p010_support;
  u32 format_customer1_support;

  u32 force_mpeg4_idct;
  u32 ch8_pix_ileav_output;

  u32 ref_buffer_test_mode_offset_enable;
  i32 ref_buffer_test_mode_offset_min;
  i32 ref_buffer_test_mode_offset_max;
  i32 ref_buffer_test_mode_offset_start;
  i32 ref_buffer_test_mode_offset_incr;

  u32 apf_disable;
  u32 apf_threshold_disable;
  i32 apf_threshold_value;

  u32 tiled_ref_support;
  u32 stride_support;
  i32 field_dpb_support;
  i32 addr64_support;

  u32 service_merge_disable;

  u32 strm_swap;
  u32 pic_swap;
  u32 dirmv_swap;
  u32 tab0_swap;
  u32 tab1_swap;
  u32 tab2_swap;
  u32 tab3_swap;
  u32 rscan_swap;
  u32 comp_tab_swap;
  u32 max_burst;
  u32 ref_double_buffer_enable;

  u32 timeout_cycles;

  u32 axi_id_rd;
  u32 axi_id_rd_unique_enable;
  u32 axi_id_wr;
  u32 axi_id_wr_unique_enable;
  u32 cache_support;
  u32 muti_core_support;
  u32 axi_wr_outstand;
  u32 axi_rd_outstand;
};

struct TBPpParams {
  char output_picture_endian[14];
  char input_picture_endian[14];
  char word_swap[9];
  char word_swap16[9];
  u32 bus_burst_length;
  char clock_gating[9];
  char data_discard[9];
  char multi_buffer[9];

  u32 max_pp_out_pic_width;
  u32 ppd_exists;
  u32 dithering_support;
  u32 scaling_support;
  u32 deinterlacing_support;
  u32 alpha_blending_support;
  u32 ablend_crop_support;
  u32 pp_out_endian_support;
  u32 tiled_support;
  u32 tiled_ref_support;

  i32 fast_hor_down_scale_disable;
  i32 fast_ver_down_scale_disable;
  i32 vert_down_scale_stripe_disable_support;
  u32 pix_acc_out_support;

  u32 filter_enabled;   /* Enable deblocking filter for MPEG4. */
  u32 pipeline_e;       /* PP pipeline mode enabled */
  u32 tiled_e;          /* Tiled output enabled */
  u32 in_width;         /* PP standalone input width/height in pixel */
  u32 in_height;
  u32 in_stride;        /* PP standalone input stride in bytes */
  u32 pre_fetch_height; /* PP standalone prefetch reading height: 16 or 64? */
};

/* PP units params */
struct TBPpUnitParams {
  u32 unit_enabled;
  u32 tiled_e;          /* Tiled output enabled */
  u32 rgb;
  u32 rgb_planar;
  u32 monochrome;       /* PP output luma only */
  u32 cr_first;         /* CrCb instead of CbCr */
  u32 out_p010;         /* P010 output */
  u32 out_1010;
  u32 out_I010;
  u32 out_L010;
  u32 out_cut_8bits;      /* Cut to 8-bit output */
  /* crop input */
  u32 crop_x;
  u32 crop_y;
  u32 crop_width;
  u32 crop_height;
  /* second crop input*/
  u32 crop2_x;
  u32 crop2_y;
  u32 crop2_width;
  u32 crop2_height;
  /* scale out */
  u32 scale_width;
  u32 scale_height;
  u32 shaper_enabled;
  u32 shaper_no_pad;
  u32 dec400_enabled;
 
  u32 planar;
  u32 ystride;        /* Stride for Y/C plane */
  u32 cstride;
  u32 align;          /* alignment for this pp output channel */
  char tiled_mode[12];
};

#ifdef ASIC_TRACE_SUPPORT
/*rtl block check enable params*/
struct RtlchkParams {
  u8 emd_tu_ctrl;
  u8 emd_cu_ctrl;
  u8 cabac_coeffs;
  u8 dmv_ctrl;
  u8 alf_ctrl;
  u8 iqt_tu_ctrl;
  u8 iqt_out;
  u8 mvd_cu_ctrl;
  u8 mv_ctrl;
  u8 mvd_out_dir_mvs;
  u8 pft_cu_ctrl      ;
  u8 mv_ctrl_to_pred  ;
  u8 apf_transact     ;
  u8 apf_part_ctrl    ;
  u8 inter_out_mv_ctrl;
  u8 inter_pred_out   ;
  u8 intra_cu_ctrl      ;
  u8 filterd_ctrl       ;
  u8 pred_out           ;
  u8 filterd_out        ;
  u8 filterd_out_blkctrl;
  u8 sao_out_ctrl       ;
  u8 sao_out_data       ;
  u8 alf_out_pp_ctrl    ;
  u8 alf_out_pp_data    ;
  u8 edc_cbsr_burst_ctrl;
  u8 miss_table_trans   ;
  u8 update_table_trans ;
};
#endif

struct CmdbufParams {
  u8 cmd_en;
};

struct AxiParams {
  u8 rd_axi_id;
  u8 axi_rd_id_e;
};

struct TBCfg {
  struct TBParams tb_params;
  struct TBDecParams dec_params;
  struct TBPpParams pp_params;
  struct TBPpUnitParams pp_units_params[DEC_MAX_PPU_COUNT];
#ifdef ASIC_TRACE_SUPPORT
  struct RtlchkParams rtlchk_params;
#endif
  struct CmdbufParams cmdbuf_params;
  struct AxiParams axi_params; 

  u32    ppu_index;
  u32 shaper_bypass;
  u32 cache_enable;
  u32 shaper_enable;
};

#endif /* TB_DEFS_H */
