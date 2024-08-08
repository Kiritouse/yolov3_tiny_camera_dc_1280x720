/*
 * MIPI capture test head file
 *
 * Maintainer: Yiping.Liang <Yiping.Liang@orbita.com>
 *
 * Copyright (C) 2021 Orbita Inc.
 *
 */

#ifndef __OPENCV_UTILS_H__
#define __OPENCV_UTILS_H__

#include <stdint.h>

#define OPENCV_UTILS_LOG_BUF_LEN	128


typedef enum {
	OPENCV_UTILS_DEBUG = 0,
	OPENCV_UTILS_INFO,
	OPENCV_UTILS_WARN,
	OPENCV_UTILS_ERROR,
} opencv_utils_log_t;

typedef int (*opencv_utils_log_func_t)(opencv_utils_log_t type, const char* msg);
                                  
typedef struct {
	int 	x;
	int 	y;
} point_t;


int opencv_point_object_nv12(uint8_t* nv12, uint32_t width, uint32_t height, 
									float r, float g, float b,
									point_t pt1, point_t pt2, 
									const char* object_name);
int opencv_utils_set_log_func(opencv_utils_log_func_t log);
int opencv_add_txt_nv12(uint8_t* nv12, uint32_t width, uint32_t height, 
                                    float r, float g, float b,
                                    point_t pt, const char* text);

#endif /* __OPENCV_UTILS_H__) */
