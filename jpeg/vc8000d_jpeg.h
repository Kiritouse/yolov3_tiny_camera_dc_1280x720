/*
 * VC8000D jpeg decode test head file
 *
 * Maintainer: Renwei.Liu <Renwei.Liu@orbita.com>
 *
 * Copyright (C) 2020 Orbita Inc.
 *
 */

#ifndef __VC8000D_JPEG_H__
#define __VC8000D_JPEG_H__

#include <stdint.h>
#include "basetype.h"

#define JPEGDEC_INBUF_NUM_MAX	16
#define JPEGDEC_OUTBUF_NUM_MAX	16
#define JPEGDEC_LOG_BUF_LEN	128

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	JPEGDEC_DEBUG = 0,
	JPEGDEC_INFO,
	JPEGDEC_WARN,
	JPEGDEC_ERR,
} jpegdec_log_t;

typedef int (*jpegdec_log_func_t)(jpegdec_log_t type, const char* msg);

typedef struct {
	uint32_t width;
	uint32_t height;
	uint32_t scale_enabled;
	uint32_t scaled_width;
	uint32_t scaled_height;
	uint32_t output_fmt; /* 0 - nv12; 1 - rgb888*/
	uint32_t input_buf_num;
	uint32_t output_buf_num;
	jpegdec_log_func_t 	log;
}jpegdec_config_t;

typedef struct {
	uint32_t	*virtual_addr;
	addr_t		bus_address;
	uint32_t	size;
	uint32_t 	width;
	uint32_t 	height;
	uint32_t	stride;
	uint32_t    stride_ch;
}jpegdec_output_t;

typedef struct {
	uint32_t	*virtual_addr;
	addr_t		bus_address;
	uint32_t	size;
}jpegdec_input_t;


const void* jpegdec_open(jpegdec_config_t * config);
int jpegdec_decode(const void * jpegdec_handle, jpegdec_input_t * jpeg_input, jpegdec_output_t * jpeg_output);
void jpegdec_close(const void * jpegdec_handle);
int queue_output_buf(const void * jpegdec_handle, jpegdec_output_t jpeg_output);


#ifdef __cplusplus
}
#endif

#endif /* __VC8000D_JPEG_H__ */

