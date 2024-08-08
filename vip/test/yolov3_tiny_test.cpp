/*
 * yolov3 tiny nbg test source file
 *
 * Maintainer: Yiping.Liang <Yiping.Liang@orbita.com>
 *
 * Copyright (C) 2021 Orbita Inc.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>
#include "yolov3_tiny.h"

static int log_level;


#define BILLION                                 1000000000
static uint64_t get_perf_count()
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (uint64_t)((uint64_t)ts.tv_nsec + (uint64_t)ts.tv_sec * BILLION);
}


static int yolov3_tiny_print_custom(yolov3_tiny_log_t type, const char* msg)
{
	if(type >= log_level)
		printf("[yolov3_tiny] %s\n", msg);
	return 0;
}


static void usage(void)
{
	printf("Usage: yolov3 tiny test [option] ..\n");
	printf("\t -i input image file name \n");
	printf("\t -w input image width \n");
	printf("\t -h input image height \n");
	printf("\t -f input image format \n");
	printf("\t -d yolov3 tiny network binary file path \n");
	printf("\t -l log level 0~3 \n");
}


int main(int argc, char **argv)
{
	int opt;
	FILE *fin;
	int len;
	uint8_t *frame1, *frame2, *frame3, *frame4;
	int ret;
	int cnt;
	user_detected_list list;
	yolov3_tiny_config_t config = {0};
	uint64_t tmsStart, tmsEnd, msVal, usVal;
	int frame_cnt;
	
	if(argc < 2){
		usage();
		exit(0);
	}

	while ((opt = getopt(argc, argv, "w:h:f:i:l:d:")) != -1){
		switch (opt) {
		case 'i':
			fin = fopen(optarg, "rb");
			if (!fin) {
				printf("open nv12 file %s fail\n", optarg);
				return -1;
			}
			break;
		case 'l':
			log_level = atoi(optarg);
			break;
		case 'w':
			config.width = atoi(optarg);
			break;
		case 'h':
			config.height = atoi(optarg);
			break;
		case 'f':
			config.format = atoi(optarg);;
			break;
		case 'd':
			strcpy((char*)config.data_name, optarg);
			break;
		default:
			usage();
			return -1;
		}
	}

	//lyp，读文件
	switch(config.format){
	case 0:
		len = config.width*config.height*3/2;
		break;
	case 1:
		len = config.width*config.height*3;
		break;
	default:
		printf("config format err: %u\n", config.format);
		exit(0);
	}
	frame1 = (uint8_t*)malloc(len);
	frame2 = (uint8_t*)malloc(len);
	frame3 = (uint8_t*)malloc(len);
	frame4 = (uint8_t*)malloc(len);
	fread(frame1, sizeof(uint8_t), len, fin);
	memcpy(frame2, frame1, len);
	memcpy(frame3, frame1, len);
	memcpy(frame4, frame1, len);

	//lyp，初始化vip
	config.log = yolov3_tiny_print_custom;
	ret = yolov3_tiny_init(&config);
	if(ret){
		printf("yolov3_tiny_init failed: %d\n", ret);
		goto exit;
	}

	tmsStart = get_perf_count();
	//lyp，检测4帧图像
	ret = yolov3_tiny_process(frame1, 1);
	if(ret){
		printf("yolov3_tiny_process frame 1 failed: %d\n", ret);
		goto exit;
	}
	yolov3_tiny_process(frame2, 2);
	if(ret){
		printf("yolov3_tiny_process frame 2 failed: %d\n", ret);
		goto exit;
	}
	yolov3_tiny_process(frame3, 3);
	if(ret){
		printf("yolov3_tiny_process frame 3 failed: %d\n", ret);
		goto exit;
	}
	yolov3_tiny_process(frame4, 4);
	if(ret){
		printf("yolov3_tiny_process frame 4 failed: %d\n", ret);
		goto exit;
	}

	cnt = 0;
	frame_cnt = 0;
	//lyp，检测一帧的时间约为30ms，4帧检测时间约为120ms，运行一定时间退出即可
	while(cnt < 1000){
		ret = yolov3_tiny_get_result_async(&list);
		if(ret == 0){
			for(int i = 0; i < list.detected_class_num; i++){
				printf("frame %d detect %s(%d) : left_top_x(%d) left_top_y(%d) right_bot_x(%d) right_bot_y(%d) prob(%f) in image %ux%u\n", 
					list.frame_id, list.obj[i].name, list.obj[i].name_id,
					list.obj[i].left_top_x, list.obj[i].left_top_y, 
					list.obj[i].right_bot_x, list.obj[i].right_bot_y, list.obj[i].prob, 
					list.image_scaled_width, list.image_scaled_height);
			}
			if(++frame_cnt >= 4) break;
		}else{
//			printf("no obj detected\n");
//			usleep(30*1000);
			usleep(1000);
		}
//		printf("cnt = %d\n", cnt++);
	}
	tmsEnd = get_perf_count();
	msVal = (tmsEnd - tmsStart)/1000000;
    usVal = (tmsEnd - tmsStart)/1000;
    printf("yolov3 tiny: frame cnt: %d costed %llums or %lluus\n", frame_cnt, msVal, usVal);

exit:
	free(frame1);
	free(frame2);
	free(frame3);
	free(frame4);
	fclose(fin);
	yolov3_tiny_release();
	return 0;
	
}

