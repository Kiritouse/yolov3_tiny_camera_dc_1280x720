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

#ifndef __DWL_H__
#define __DWL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "basetype.h"
#include "decapicommon.h"
#include "dwlthread.h"
#include "vpufeature.h"

enum DWLCoreStatus {
  DWL_OK = 0,
  DWL_ERROR = -1,
  DWL_HW_WAIT_TIMEOUT = 1
};

/**
 * @enum DWLDMADirection
 * @brief   DMA transfer direction
 */
enum DWLDMADirection {
  HOST_TO_DEVICE = 0, /**< copy the data from host memory to device memory */
  DEVICE_TO_HOST = 1  /**< copy the data from device memory to host memory */
};

#define DWL_OK 0
#define DWL_ERROR -1

#define DWL_CACHE_OK  0
#define DWL_CACHE_FATAL_RECOVERY 1
#define DWL_CACHE_FATAL_UNRECOVERY 2

#define DWL_HW_WAIT_OK DWL_OK
#define DWL_HW_WAIT_ERROR DWL_ERROR
#define DWL_HW_WAIT_TIMEOUT 1

#define DWL_CLIENT_TYPE_H264_DEC 1U
#define DWL_CLIENT_TYPE_MPEG4_DEC 2U
#define DWL_CLIENT_TYPE_JPEG_DEC 3U
#define DWL_CLIENT_TYPE_PP 4U
#define DWL_CLIENT_TYPE_VC1_DEC 5U
#define DWL_CLIENT_TYPE_MPEG2_DEC 6U
#define DWL_CLIENT_TYPE_VP6_DEC 7U
#define DWL_CLIENT_TYPE_AVS_DEC 8U
#define DWL_CLIENT_TYPE_RV_DEC 9U
#define DWL_CLIENT_TYPE_VP8_DEC 10U
#define DWL_CLIENT_TYPE_VP9_DEC 11U
#define DWL_CLIENT_TYPE_HEVC_DEC 12U
#define DWL_CLIENT_TYPE_ST_PP 14U
#define DWL_CLIENT_TYPE_H264_MAIN10 15U
#define DWL_CLIENT_TYPE_AVS2_DEC 16U
#define DWL_CLIENT_TYPE_AV1_DEC 17U

#define DWL_MEM_TYPE_CPU                 0x0000U /* CPU RW. non-secure CMA memory */
#define DWL_MEM_TYPE_SLICE               0x0001U /* VPU R, CAAM W */
#define DWL_MEM_TYPE_DPB                 0x0002U /* VPU RW, Render R */
#define DWL_MEM_TYPE_VPU_WORKING         0x0003U /* VPU R, CPU W. non-secure memory */
#define DWL_MEM_TYPE_VPU_WORKING_SPECIAL 0x0004U /* VPU R, CPU RW. only for VP9 counter context table */
#define DWL_MEM_TYPE_VPU_ONLY            0x0005U /* VPU RW only. */
#define DWL_MEM_TYPE_VPU_CPU             0x0006U /* VPU&CPU RW. */

#define DWL_MEM_TYPE_DMA_DEVICE_ONLY     0x0000U /**< only host or noly device use the buffer */
#define DWL_MEM_TYPE_DMA_HOST_TO_DEVICE  0x0100U /**< the buffer need transfer frome host to device */
#define DWL_MEM_TYPE_DMA_DEVICE_TO_HOST  0x0200U /**< the buffer need transfer frome device to host */
#define DWL_MEM_TYPE_DMA_HOST_AND_DEVICE 0x0300U /**< the buffer need transfer in both directions */

#ifdef MEM_ONLY_DEV_CHECK
#define DWL_DEVMEM_INIT                    0
#define DWL_DEVMEM_VAILD(mem)              ((mem).bus_address != 0)
#define DWL_DEVMEM_COMPARE(mem, addr)      ((mem).bus_address == addr)
#define DWL_DEVMEM_COMPARE_V2(mem1, mem2)  ((mem1).bus_address == (mem2).bus_address)
#define DWL_GET_DEVMEM_ADDR(mem)           ((mem).bus_address)
typedef addr_t DWLMemAddr;
#else
#define DWL_DEVMEM_INIT                    NULL
#define DWL_DEVMEM_VAILD(mem)              ((mem).virtual_address != NULL)
#define DWL_DEVMEM_COMPARE(mem, addr)      ((mem).virtual_address == addr)
#define DWL_DEVMEM_COMPARE_V2(mem1, mem2)  ((mem1).virtual_address == (mem2).virtual_address)
#define DWL_GET_DEVMEM_ADDR(mem)           ((mem).virtual_address)
typedef const u32* DWLMemAddr;
#endif

/**
 * \struct DWLLinearMem
 * \brief Linear memory area descriptor.
 * \ingroup common_group
 */
struct DWLLinearMem {
  u32 *virtual_address;
  addr_t bus_address;
  u32 size;         /**< physical size (rounded to page multiple) */
  u32 logical_size; /**< requested size in bytes */
  u32 mem_type;
#ifdef SUPPORT_MMU
  addr_t unmap_bus_address; /**< used when free buffer*/
#endif
};

/* DWLInitParam is used to pass parameters when initializing the DWL */
struct DWLInitParam {
  u32 client_type;
};

/* Hardware configuration description, same as in top API */
typedef struct DecHwConfig DWLHwConfig;

struct DWLHwFuseStatus {
  u32 vp6_support_fuse;            /* HW supports VP6 */
  u32 vp7_support_fuse;            /* HW supports VP7 */
  u32 vp8_support_fuse;            /* HW supports VP8 */
  u32 vp9_support_fuse;            /* HW supports VP9 */
  u32 h264_support_fuse;           /* HW supports H.264 */
  u32 HevcSupportFuse;             /* HW supports HEVC */
  u32 mpeg4_support_fuse;          /* HW supports MPEG-4 */
  u32 mpeg2_support_fuse;          /* HW supports MPEG-2 */
  u32 sorenson_spark_support_fuse; /* HW supports Sorenson Spark */
  u32 jpeg_support_fuse;           /* HW supports JPEG */
  u32 vc1_support_fuse;            /* HW supports VC-1 Simple */
  u32 jpeg_prog_support_fuse;      /* HW supports Progressive JPEG */
  u32 pp_support_fuse;             /* HW supports post-processor */
  u32 pp_config_fuse;              /* HW post-processor functions bitmask */
  u32 max_dec_pic_width_fuse;      /* Maximum video decoding width supported  */
  u32 max_pp_out_pic_width_fuse;   /* Maximum output width of Post-Processor */
  u32 ref_buf_support_fuse;        /* HW supports reference picture buffering */
  u32 avs_support_fuse;            /* HW supports AVS */
  u32 rv_support_fuse;             /* HW supports RealVideo codec */
  u32 mvc_support_fuse;            /* HW supports MVC */
  u32 custom_mpeg4_support_fuse;   /* Fuse for custom MPEG-4 */
};


/* DWLCodecConfig is used for IP-integration codec config. */
struct DWLCodecConfig {
  // Set swaps for encoder frame input data. From MSB to LSB: swap each
  // 128 bits, 64, 32, 16, 8.
  u32 input_yuv_swap;
  // Set swaps for encoder frame output (stream) data. From MSB to LSB:
  // swap each 128 bits, 64, 32, 16, 8.
  u32 output_swap;
  // Set swaps for encoder probability tables. From MSB to LSB: swap each
  // 128 bits, 64, 32, 16, 8.
  u32 prob_table_swap;
  // Set swaps for encoder context counters. From MSB to LSB: swap each
  // 128 bits, 64, 32, 16, 8.
  u32 ctx_counter_swap;
  // Set swaps for encoder statistics. From MSB to LSB: swap each 128
  // bits, 64, 32, 16, 8.
  u32 statistics_swap;
  // Set swaps for encoder foreground/background map. From MSB to LSB:
  // swap each 128 bits, 64, 32, 16, 8.
  u32 fgbg_map_swap;

  // ASIC interrupt enable.
  // This enables/disables the ASIC to generate interrupt.
  bool irq_enable;

  // AXI master read ID base. Read service type specific values are added
  // to this.
  u32 axi_read_id_base;
  // AXI master write id base. Write service type specific values are
  // added to this.
  u32 axi_write_id_base;
  // AXI maximum read burst issued by the core [1..511]
  u32 axi_read_max_burst;
  // AXI maximum write burst issued by the core [1..511]
  u32 axi_write_max_burst;

  // ASIC clock gating enable.
  // This enables/disables the ASIC to utilize clock gating.
  // If this is 'true', ASIC core clock is automatically shut down when
  // the encoder enable bit is '0' (between frames).
  bool clock_gating_enabled;

  // ASIC internal timeout period in clocks.
  // Timeout counter acts as a watchdog that asserts a timeout interrupt
  // if bus is idle for the set period while the encoder is enabled.
  // Set to zero to disable.
    u32 timeout_period;
};


struct strmInfo {
  u32 low_latency;
  u32 send_len;
  addr_t strm_bus_addr;
  addr_t strm_bus_start_addr;
  u8* strm_vir_addr;
  u8* strm_vir_start_addr;
  u32 last_flag;
};
struct CacheParams {
  u64 buf_size;
  u32 dpb_size;
};
/* HW ID retrieving, static implementation */
u32 DWLReadAsicID(u32 client_type);
u32 DWLReadCoreAsicID(u32 core_id);
/* HW Build ID is for feature list query */
u32 DWLReadHwBuildID(u32 client_type);
u32 DWLReadCoreHwBuildID(u32 core_id);
/* improve the cpu cycle, don't use the GetReleaseHwFeaturesById with memcpy directly */
u32 DWLGetReleaseHwFeaturesByClientType(u32 client_type, const struct DecHwFeatures **hw_feature);

/* HW configuration retrieving, static implementation */
void DWLReadAsicConfig(DWLHwConfig *hw_cfg, u32 client_type);
void DWLReadMCAsicConfig(DWLHwConfig hw_cfg[MAX_ASIC_CORES]);

/* Return number of ASIC cores, static implementation */
u32 DWLReadAsicCoreCount(void);

/* HW fuse retrieving, static implementation */
void DWLReadAsicFuseStatus(struct DWLHwFuseStatus *hw_fuse_sts);

/* DWL initialization and release */
const void *DWLInit(struct DWLInitParam *param);
i32 DWLRelease(const void *instance);

/* HW sharing */
i32 DWLReserveHw(const void *instance, i32 *core_id, u32 client_type);
i32 DWLReserveHwPipe(const void *instance, i32 *core_id);
u32 DWLReleaseHw(const void *instance, i32 core_id);
void DWLReleaseL2(const void *instance, i32 core_id, u32 irq_buffer);

/* Frame buffers memory */
i32 DWLMallocRefFrm(const void *instance, u32 size, struct DWLLinearMem *info);
void DWLFreeRefFrm(const void *instance, struct DWLLinearMem *info);

/* SW/HW shared memory */
i32 DWLMallocLinear(const void *instance, u32 size, struct DWLLinearMem *info);
void DWLFreeLinear(const void *instance, struct DWLLinearMem *info);

/* Register access */
void DWLWriteReg(const void *instance, i32 core_id, u32 offset, u32 value);
u32 DWLReadReg(const void *instance, i32 core_id, u32 offset);

/* HW starting/stopping */
void DWLEnableHw(const void *instance, i32 core_id, u32 offset, u32 value);
void DWLDisableHw(const void *instance, i32 core_id, u32 offset, u32 value);

/* HW register access */
void DWLWriteRegToHw(const void *instance, i32 core_id, u32 offset, u32 value);
u32 DWLReadRegFromHw(const void *instance, i32 core_id, u32 offset);

/* HW synchronization */
i32 DWLWaitHwReady(const void *instance, i32 core_id, u32 timeout);

typedef void DWLIRQCallbackFn(void *arg, i32 core_id);

void DWLSetIRQCallback(const void *instance, i32 core_id,
                       DWLIRQCallbackFn *callback_fn, void *arg);
void DWLReadPpConfigure(const void *instance, u32 core_id, void *ppu_cfg, u16 *tiles, u32 tile_enable);
void DWLReadMcRefBuffer(const void *instance, u32 core_id, u64 buf_size, u32 dpb_size);
/* SW/SW shared memory */
void *DWLmalloc(u32 n);
void DWLfree(void *p);
void *DWLcalloc(size_t n, size_t s);
void *DWLmemcpy(void *d, const void *s, u32 n);
void *DWLmemset(void *d, i32 c, u32 n);

/* VCMD operation */
u32 DWLVcmdCores(void);
i32 DWLReserveCmdBuf(const void *instance, u32 client_type, u32 width, u32 height, u32 *cmd_buf_id);
i32 DWLEnableCmdBuf(const void *instance, u32 cmd_buf_id);
i32 DWLWaitCmdBufReady(const void *instance, u16 cmd_buf_id);
i32 DWLReleaseCmdBuf(const void *instance, u32 cmd_buf_id);
i32 DWLWaitCmdbufsDone(const void *instance);
i32 DWLFlushRegister(const void *instance, u32 cmd_buf_id, u32 *dec_regs, u32 *mc_fresh_regs, u32 mc_buf_id);
i32 DWLRefreshRegister(const void *instance, u32 cmd_buf_id, u32 *dec_regs);
i32 DWLMCCallBackFlush(const void *instance, u32 *dec_regs, u32 cmdbuf_id);
i32 DWLGetMCCoreId(const void *instance, u32 cmdbuf_id);
u32 *DWLGetCmdRegMirror(const void *instance, u32 cmdbuf_id);
void **DWLGetCmdCacheCtx(const void *instance, u32 cmdbuf_id);

/* SW/HW shared memory access*/
typedef u8 DWLReadByteFn(const u8 *p, u32 size);
u8 DWLNoLatencyReadByte(const u8 *p, u32 size);
u8 DWLPrivateAreaReadByte(const u8 *p);
u8 DWLLowLatencyReadByte(const u8 *p, u32 buf_size);
DWLReadByteFn *DWLGetReadByteFunc(void);
void DWLPrivateAreaWriteByte(u8 *p, u8 data);
void * DWLPrivateAreaMemcpy(void *d,  const void *s,  u32 n);
void * DWLPrivateAreaMemset(void *p,  i32 c, u32 n);

void DWLDMATransData(addr_t device_bus_addr, void *host_virtual_addr,
                     u32 length, enum DWLDMADirection dir);

/* Decoder wrapper layer functionality. */
#ifdef _HAVE_PTHREAD_H
struct DWL {
  /* HW sharing */
  i32 (*ReserveHw)(const void *instance, i32 *core_id, u32 client_type);
  i32 (*ReserveHwPipe)(const void *instance, i32 *core_id);
  u32 (*ReleaseHw)(const void *instance, i32 core_id);
  /* Physical, linear memory functions */
  i32 (*MallocLinear)(const void *instance, u32 size,
                      struct DWLLinearMem *info);
  void (*FreeLinear)(const void *instance, struct DWLLinearMem *info);
  /* Register access */
  void (*WriteReg)(const void *instance, i32 core_id, u32 offset, u32 value);
  u32 (*ReadReg)(const void *instance, i32 core_id, u32 offset);
  /* HW starting/stopping */
  void (*EnableHw)(const void *instance, i32 core_id, u32 offset, u32 value);
  void (*DisableHw)(const void *instance, i32 core_id, u32 offset, u32 value);
  /* HW synchronization */
  i32 (*WaitHwReady)(const void *instance, i32 core_id, u32 timeout);
  void (*SetIRQCallback)(const void *instance, i32 core_id,
                         DWLIRQCallbackFn *callback_fn, void *arg);
  /* Virtual memory functions. */
  void *(*Malloc)(size_t n);
  void (*Free)(void *p);
  void *(*Calloc)(size_t n, size_t s);
  void *(*Memcpy)(void *d, const void *s, size_t n);
  void *(*Memset)(void *d, i32 c, size_t n);
  /* POSIX compatible threading functions. */
  i32 (*Pthread_create)(pthread_t *tid, const pthread_attr_t *attr,
                        void *(*start)(void *), void *arg);
  void (*Pthread_exit)(void *value_ptr);
  i32 (*Pthread_join)(pthread_t thread, void **value_ptr);
  i32 (*Pthread_mutex_init)(pthread_mutex_t *mutex,
                            const pthread_mutexattr_t *attr);
  i32 (*Pthread_mutex_destroy)(pthread_mutex_t *mutex);
  i32 (*Pthread_mutex_lock)(pthread_mutex_t *mutex);
  i32 (*Pthread_mutex_unlock)(pthread_mutex_t *mutex);
  i32 (*Pthread_cond_init)(pthread_cond_t *cond,
                           const pthread_condattr_t *attr);
  i32 (*Pthread_cond_destroy)(pthread_cond_t *cond);
  i32 (*Pthread_cond_wait)(pthread_cond_t *cond, pthread_mutex_t *mutex);
  i32 (*Pthread_cond_signal)(pthread_cond_t *cond);
  /* API trace function. Set to NULL if no trace wanted. */
  int (*Printf)(const char *string, ...);
};
#endif
#ifdef __cplusplus
}
#endif

#endif /* __DWL_H__ */
