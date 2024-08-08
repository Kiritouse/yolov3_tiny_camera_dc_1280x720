/*
 * yolov3 tiny nbg test head file
 *
 * Maintainer: Yiping.Liang <Yiping.Liang@orbita.com>
 *
 * Copyright (C) 2021 Orbita Inc.
 *
 */


#ifndef __YOLOV3_TINY_H__
#define __YOLOV3_TINY_H__

#include "stdint.h"
#include "yolov3_tiny_global.h"

#define YOLOV3_TINY_LOG_BUF_LEN		128
#define YOLOV3_TINY_GRAPH_NUM		2

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	YOLOV3_TINY_DEBUG = 0,
	YOLOV3_TINY_INFO,
	YOLOV3_TINY_WARN,
	YOLOV3_TINY_ERR,
} yolov3_tiny_log_t;

typedef int (*yolov3_tiny_log_func_t)(yolov3_tiny_log_t type, const char* msg);

typedef struct {
	yolov3_tiny_log_func_t 		log;
	const char 					data_name[128];
	uint32_t					width;
	uint32_t					height;
	uint8_t						format;	/* 0: NV12; 1: rgb888 */
}yolov3_tiny_config_t;

int yolov3_tiny_init(yolov3_tiny_config_t* config);
int yolov3_tiny_process(uint8_t* frame, int frame_id);
int yolov3_tiny_get_result_async(user_detected_list* list);
void yolov3_tiny_release();
double get_process_graph_time_ms(int id);
int get_process_graph_num();

#ifdef __cplusplus
}
#endif

#endif /* __YOLOV3_TINY_H__ */

