/*
 * MIPI capture test head file
 *
 * Maintainer: Yiping.Liang <Yiping.Liang@orbita.com>
 *
 * Copyright (C) 2021 Orbita Inc.
 *
 */

#ifndef __CAMERA_CAP_USH_H__
#define __CAMERA_CAP_USH_H__

#include <stdint.h>

#define CAMERA_USB_OUT_BUF_MAX	4
#define CAMERA_USB_LOG_BUF_LEN	128

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	CAMERA_USB_DEBUG = 0,
	CAMERA_USB_INFO,
	CAMERA_USB_WARN,
	CAMERA_USB_ERROR,
} camera_usb_log_t;

typedef int (*camera_log_func_t)(camera_usb_log_t type, const char* msg);
                                  
typedef struct {
	uint8_t 	*virt_addr;
	uint32_t 	phys_addr;
	uint32_t 	len;
	uint32_t	index;
} camera_usb_outbuf_t;


typedef struct {
	uint32_t			width;
	uint32_t 			height;
	uint32_t			phys_addr[CAMERA_USB_OUT_BUF_MAX];
	uint32_t			buf_num;
	uint32_t 			format;	/* 0: YUYV	1: MJPEG*/
	camera_log_func_t 	log;
} camera_usb_config_t;


int camera_usb_open(char *dev_name, camera_usb_config_t *config);
int camera_usb_start();
int camera_usb_stop();
int camera_usb_close();
int camera_usb_get_outbuf(camera_usb_outbuf_t *outbuf);
int camera_usb_prepare_outbuf(camera_usb_outbuf_t *outbuf);
int camera_usb_get_fps();

#ifdef __cplusplus
}
#endif

#endif /* __CAMERA_CAP_USH_H__ */