/*
 * DC8000 test head file
 *
 * Maintainer: Renwei.Liu <Renwei.Liu@orbita.com>
 *
 * Copyright (C) 2020 Orbita Inc.
 *
 */

#ifndef __DC8000_H__
#define __DC8000_H__

#include <stdint.h>
#include "basetype.h"

#define DC8000_LOG_BUF_LEN	128
#define DC8000_DISPLAY_FPS	60 /*最大不超过60*/

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	DC8000_DEBUG = 0,
	DC8000_INFO,
	DC8000_WARN,
	DC8000_ERR,
} dc8000_log_t;

typedef int (*dc8000_log_func_t)(dc8000_log_t type, const char* msg);

i32 dc8000_open(u32 width, u32 height, dc8000_log_func_t log_func);
void dc8000_close();
void dc8000_display_async(addr_t y_addr, addr_t uv_addr, int addr_id);
int dc8000_get_display_done_addr_id();
//void dc8000_obt_update_overlay(addr_t y_addr, addr_t uv_addr);
i32 mmap_image_buffer(addr_t *buf_addr, addr_t phy_addr, i32 size);
i32 munmap_image_buffer(addr_t *buf_addr, i32 size);
u32 dc8000_display_fps();

#ifdef __cplusplus
}
#endif

#endif /* __DC8000_H__ */

