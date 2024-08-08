/*
 * camer capture test source file
 *
 * Maintainer: Yiping.Liang <Yiping.Liang@orbita.com>
 *
 * Copyright (C) 2021 Orbita Inc.
 *
 */
 
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include "opencv_utils.h"


static int log_level;


#define BILLION                                 1000000000
static uint64_t get_perf_count()
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (uint64_t)((uint64_t)ts.tv_nsec + (uint64_t)ts.tv_sec * BILLION);
}


static int opencv_utils_custom(opencv_utils_log_t type, const char* msg)
{
	if(type >= log_level)
		printf("[opencv_utils] %s\n", msg);
	return 0;
}


static void usage(void)
{
	printf("Usage: opencv test [option] ..\n");
	printf("\t -w nv12 image width \n");
	printf("\t -h nv12 image height \n");
	printf("\t -i input nv12 file name \n");
	printf("\t -l log level 0~3 \n");
}


int main(int argc, char *argv[])
{
	uint32_t w,h;
	int opt;
	FILE *fin, *fout;
	char file_name[64] = {0};
	uint64_t tmsStart, tmsEnd, msVal, usVal;

	if(argc < 2){
		usage();
		exit(0);
	}

	while ((opt = getopt(argc, argv, "w:h:i:l:")) != -1){
		switch (opt) {
		case 'w':
			w = atoi(optarg);
			break;
		case 'h':
			h = atoi(optarg);
			break;
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
		default:
			usage();
			return -1;
		}
	}

	//lyp，设置log函数
	opencv_utils_set_log_func(opencv_utils_custom);

	//lyp，读nv12文件
	int len = w*h*3/2;
	uint8_t *nv12 = (uint8_t*)malloc(len);
	fread(nv12, sizeof(uint8_t), len, fin);
	fflush(fin);
	fclose(fin);

	tmsStart = get_perf_count();
	//lyp，标注狗，红
	const char* dog = "dog: 77.8%";
	point_t pt1 = {58, 134};
	point_t pt2 = {214, 400};
	opencv_point_object_nv12(nv12, w, h, 255, 0, 0, pt1, pt2, dog);
	//lyp，标注车，绿
	const char* car = "car: 62.6%";
	point_t pt3 = {226, 55};
	point_t pt4 = {398, 129};
	opencv_point_object_nv12(nv12, w, h, 0, 255, 0, pt3, pt4, car);
	//标注自行车，蓝
	const char* bicycle = "bicycle: 39.3%";
	point_t pt5 = {107, 122};
	point_t pt6 = {317, 341};
	opencv_point_object_nv12(nv12, w, h, 0, 0, 255, pt5, pt6, bicycle);
	tmsEnd = get_perf_count();
	msVal = (tmsEnd - tmsStart)/1000000;
    usVal = (tmsEnd - tmsStart)/1000;
    printf("opencv cost: %llums or %lluus\n", msVal, usVal);

	//lyp，保存图片
	snprintf(file_name, sizeof(file_name), "opencv_object_%ux%u.nv12", w, h);
	fout = fopen(file_name, "wb");
	if (!fout) {
		printf("open out file %s fail\n", file_name);
	}else{
		fwrite(nv12, sizeof(uint8_t), len , fout);
		fflush(fout);
		fclose(fout);
		printf("save file: %s\n", file_name);
	}

	free(nv12);

	exit(0);

}

