/*
 * VC8000D jpeg decode test source file
 *
 * Maintainer: Renwei.Liu <Renwei.Liu@orbita.com>
 *
 * Copyright (C) 2020 Orbita Inc.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "vc8000d_jpeg.h"


static int log_level;

typedef struct{
	jpegdec_config_t dec_config;
	jpegdec_input_t  jpegdec_input;
}jpegdec_wrapper_test;


#define BILLION                                 1000000000
static uint64_t get_perf_count()
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (uint64_t)((uint64_t)ts.tv_nsec + (uint64_t)ts.tv_sec * BILLION);
}


static int jpegdec_print_custom(jpegdec_log_t type, const char* msg)
{
	if(type >= log_level)
		printf("[jpegdec] %s\n", msg);
	return 0;
}


static void usage(void)
{
	printf("Usage: jpeg decode test [option] ..\n");
	printf("\t -i input jpeg file name \n");
	printf("\t -w input jpeg width \n");
	printf("\t -h input jpeg height \n");
	printf("\t -d output scaled width/height ex: 640x480 \n");
	printf("\t -f output format: 0 - nv12; 1 - rgb888 \n");
	printf("\t -l log level 0~3 \n");
}


void* jpegdec_thread(void* arg)
{
	int ret;
	uint64_t tmsStart, tmsEnd, msVal, usVal;
	jpegdec_output_t jpegdec_output = {0};
	char file_name[32] = {0};
	FILE *fout = NULL;
	const void* jpegdec_handle;
	jpegdec_wrapper_test* wrapper = (jpegdec_wrapper_test*)arg;

	printf("jpegdec_thread enter\n");

	//lyp，修改配置为不缩放
	jpegdec_config_t dec_config;
	memcpy(&dec_config, &wrapper->dec_config, sizeof(jpegdec_config_t));
	dec_config.scale_enabled = 0;

	//lyp，打开jpeg解码器
	jpegdec_handle = jpegdec_open(&dec_config);
	if(jpegdec_handle == NULL){
		printf("jpegdec open failed\n");
		return NULL;
	}

	//lyp，jpeg解码
	tmsStart = get_perf_count();
	ret = jpegdec_decode(jpegdec_handle, &wrapper->jpegdec_input, &jpegdec_output);
	if(ret){
		printf("jpegdec decode failed: %d\n", ret);
		goto err;
	}
	tmsEnd = get_perf_count();
	msVal = (tmsEnd - tmsStart)/1000000;
    usVal = (tmsEnd - tmsStart)/1000;
    printf("jpegdec_decode: %llums or %lluus\n", msVal, usVal);

	//lyp，保存文件
	snprintf(file_name, sizeof(file_name), "jpegdec_%ux%u.%s", 
		jpegdec_output.width, jpegdec_output.height, wrapper->dec_config.output_fmt==0?"nv12":"rgb");
	fout = fopen(file_name, "wb");
	if (!fout) {
		printf("open out file %s fail\n", file_name);
		goto err;
	}
	fwrite((uint8_t*)jpegdec_output.virtual_addr, sizeof(uint8_t), jpegdec_output.size , fout);
	fflush(fout);
	fclose(fout);
	printf("save jpeg decode file: %s\n", file_name);
	if(dec_config.output_fmt == 0){
		//lyp，保存y分量
		memset(file_name, 0, sizeof(file_name));
		snprintf(file_name, sizeof(file_name), "jpegdec_%ux%u_y.nv12", jpegdec_output.width, jpegdec_output.height);
		fout = fopen(file_name, "wb");
		if (!fout) {
			printf("open out file %s fail\n", file_name);
			goto err;
		}
		fwrite((uint8_t*)jpegdec_output.virtual_addr, sizeof(uint8_t), 
				jpegdec_output.width*jpegdec_output.height, fout);
		fflush(fout);
		fclose(fout);
		printf("save jpeg decode file: %s\n", file_name);
		//lyp，保存uv分量
		memset(file_name, 0, sizeof(file_name));
		snprintf(file_name, sizeof(file_name), "jpegdec_%ux%u_uv.nv12", jpegdec_output.width, jpegdec_output.height);
		fout = fopen(file_name, "wb");
		if (!fout) {
			printf("open out file %s fail\n", file_name);
			goto err;
		}
		fwrite((uint8_t*)jpegdec_output.virtual_addr + (jpegdec_output.width*jpegdec_output.height), sizeof(uint8_t), 
				jpegdec_output.width*jpegdec_output.height/2, fout);
		fflush(fout);
		fclose(fout);
		printf("save jpeg decode file: %s\n", file_name);
	}

	//lyp，归还jpegdec_output
	queue_output_buf(jpegdec_handle, jpegdec_output);

	printf("jpegdec_thread exit\n");

err:
	jpegdec_close(jpegdec_handle);
	return NULL;

}


void* jpegdec_scaled_thread(void* arg)
{
	int ret;
	uint64_t tmsStart, tmsEnd, msVal, usVal;
	jpegdec_output_t jpegdec_output = {0};
	char file_name[32] = {0};
	FILE *fout = NULL;
	const void* jpegdec_handle;
	jpegdec_wrapper_test* wrapper = (jpegdec_wrapper_test*)arg;

	printf("jpegdec_thread enter\n");

	//lyp，修改配置为缩放
	jpegdec_config_t dec_config;
	memcpy(&dec_config, &wrapper->dec_config, sizeof(jpegdec_config_t));
	dec_config.scale_enabled = 1;

	//lyp，打开jpeg解码器
	jpegdec_handle = jpegdec_open(&dec_config);
	if(jpegdec_handle == NULL){
		printf("jpegdec open failed\n");
		return NULL;
	}

	//lyp，jpeg解码
	tmsStart = get_perf_count();
	ret = jpegdec_decode(jpegdec_handle, &wrapper->jpegdec_input, &jpegdec_output);
	if(ret){
		printf("jpegdec decode failed: %d\n", ret);
		goto err;
	}
	tmsEnd = get_perf_count();
	msVal = (tmsEnd - tmsStart)/1000000;
	usVal = (tmsEnd - tmsStart)/1000;
	printf("jpegdec_decode: %llums or %lluus\n", msVal, usVal);

	//lyp，保存文件
	snprintf(file_name, sizeof(file_name), "jpegdec_%ux%u.%s", 
		jpegdec_output.width, jpegdec_output.height, wrapper->dec_config.output_fmt==0?"nv12":"rgb");
	fout = fopen(file_name, "wb");
	if (!fout) {
		printf("open out file %s fail\n", file_name);
		goto err;
	}
	fwrite((uint8_t*)jpegdec_output.virtual_addr, sizeof(uint8_t), jpegdec_output.size , fout);
	fflush(fout);
	fclose(fout);
	printf("save jpeg decode file: %s\n", file_name);
	if(dec_config.output_fmt == 0){
		//lyp，保存y分量
		memset(file_name, 0, sizeof(file_name));
		snprintf(file_name, sizeof(file_name), "jpegdec_%ux%u_y.nv12", jpegdec_output.width, jpegdec_output.height);
		fout = fopen(file_name, "wb");
		if (!fout) {
			printf("open out file %s fail\n", file_name);
			goto err;
		}
		fwrite((uint8_t*)jpegdec_output.virtual_addr, sizeof(uint8_t), 
				jpegdec_output.width*jpegdec_output.height, fout);
		fflush(fout);
		fclose(fout);
		printf("save jpeg decode file: %s\n", file_name);
		//lyp，保存uv分量
		memset(file_name, 0, sizeof(file_name));
		snprintf(file_name, sizeof(file_name), "jpegdec_%ux%u_uv.nv12", jpegdec_output.width, jpegdec_output.height);
		fout = fopen(file_name, "wb");
		if (!fout) {
			printf("open out file %s fail\n", file_name);
			goto err;
		}
		fwrite((uint8_t*)jpegdec_output.virtual_addr + (jpegdec_output.width*jpegdec_output.height), sizeof(uint8_t), 
				jpegdec_output.width*jpegdec_output.height/2, fout);
		fflush(fout);
		fclose(fout);
		printf("save jpeg decode file: %s\n", file_name);
	}

	//lyp，归还jpegdec_output
	queue_output_buf(jpegdec_handle, jpegdec_output);

	printf("jpegdec_thread exit\n");

err:
	jpegdec_close(jpegdec_handle);
	return NULL;

}



int main(int argc, char *argv[])
{
	FILE *fin = NULL;
	uint32_t pic_num = 1;
	uint32_t w;
	uint32_t h; 
	uint32_t scaled_w = 640;
	uint32_t scaled_h = 480;
	uint32_t scale_enabled = 0; 
	uint32_t pp_enabled = 1;
	uint32_t fmt = 0;
	int opt;
	char *px;
	char tmp_str[64] = {0};
	jpegdec_wrapper_test wrapper = {0};
	uint32_t len;

	if(argc < 2){
		usage();
		exit(0);
	}

	while ((opt = getopt(argc, argv, "i:w:h:l:d:f:")) != -1){
		switch (opt) {
		case 'i':
			fin = fopen(optarg, "rb");
			if (!fin) {
				printf("open in file %s fail\n", optarg);
				return -1;
			}
			break;
		case 'w':
			w = atoi(optarg);
			break;
		case 'h':
			h = atoi(optarg);
			break;
		case 'l':
			log_level = atoi(optarg);
			break;
		case 'd':
			memcpy(tmp_str, optarg, strlen(optarg));
			px = strchr(tmp_str, 'x');
			if (!px) {
				printf("Illegal parameter: %s\n", tmp_str);
				printf("ERROR: Enable scaler parameter by using: -d[w]x[h]\n");
				return -1;
			}
			*px = '\0';
			scale_enabled = 1;
			scaled_w = atoi(tmp_str);
			scaled_h = atoi(px+1);
			if (scaled_w == 0 || scaled_h == 0) {
				printf("Illegal scaled width/height: %sx%s\n", tmp_str, px+1);
				return -1;
			}
			printf("scaled: %ux%u\n", scaled_w, scaled_h);
			break;
		case 'f':
			fmt = atoi(optarg);
			break;
		default:
			usage();
			return -1;
		}
	}

	//lyp，配置解码器参数
	wrapper.dec_config.input_buf_num = 2;
	wrapper.dec_config.output_buf_num = 2;
	wrapper.dec_config.width = w;
	wrapper.dec_config.height = h;
	wrapper.dec_config.output_fmt = fmt;
	wrapper.dec_config.scale_enabled = scale_enabled;
	wrapper.dec_config.scaled_width = scaled_w;
	wrapper.dec_config.scaled_height = scaled_h;
	wrapper.dec_config.log = jpegdec_print_custom;

	//lyp，读文件
	fseek(fin, 0L, SEEK_END);
	len = ftell(fin);
	rewind(fin);
	wrapper.jpegdec_input.size = len;
	wrapper.jpegdec_input.virtual_addr = (uint32_t*)malloc(len);
	wrapper.jpegdec_input.bus_address = 0;
	fread((uint8_t*)wrapper.jpegdec_input.virtual_addr, sizeof(uint8_t), len, fin);

	//lyp，创建两个线程，一个解码保留原尺寸，另一个解码并缩放
	pthread_t jpegdec_tid, jpegdec_scaled_tid;
	pthread_create(&jpegdec_tid, NULL, jpegdec_thread, (void*)&wrapper);
	pthread_create(&jpegdec_scaled_tid, NULL, jpegdec_scaled_thread, (void*)&wrapper);

	pthread_join(jpegdec_tid, NULL);
	pthread_join(jpegdec_scaled_tid, NULL);

	
err:
	fclose(fin);
	return 0;
}

