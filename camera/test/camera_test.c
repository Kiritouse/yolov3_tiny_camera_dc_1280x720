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
#include <linux/videodev2.h>
#include "camera_cap_usb.h"


//#define USE_PHY_ADDR
#define CAMERA_BUF_NUM 4
#define LOG_LEVEL	   1
static int log_level;
#define CAMERA_BUF_PHY_ADDR_BASE	0x10000000


static int camera_print_custom(camera_usb_log_t type, const char* msg)
{
	if(type >= log_level)
		printf("[camera] %s\n", msg);
}


static void usage(void)
{
	printf("Usage: v4l2 camera test [option] ..\n");
	printf("\t -d device id \n");
	printf("\t -w pixel width \n");
	printf("\t -h pixel height \n");
	printf("\t -n frame_nums \n");
	printf("\t -i input format \t 0 - YUYV \n");
	printf("\t                 \t 1 - MJPEG \n");
	printf("\t -l log level 0~3 \n");
}


int main(int argc, char *argv[])
{
	uint32_t w,h,n,fmt;
	int ret = 0;
	int opt;
	camera_usb_config_t config;
	char dev_name[32] = "/dev/video";
	camera_usb_outbuf_t out_buf;
	FILE* fp;
	char file_name[64];
	int index;

	if(argc < 2){
		usage();
		exit(0);
	}

	while ((opt = getopt(argc, argv, "d:w:h:i:n:l:")) != -1){
		switch (opt) {
		case 'd':
			strcat(dev_name, optarg);
			break;
		case 'w':
			w = atoi(optarg);
			break;
		case 'h':
			h = atoi(optarg);
			break;
		case 'i':
			fmt = atoi(optarg);
			break;
		case 'n':
			n = atoi(optarg);
			break;
		case 'l':
			log_level = atoi(optarg);
			break;
		default:
			usage();
			ret = -1;
			goto err;
		}
	}

	config.buf_num = CAMERA_BUF_NUM;
	config.format = fmt;
	config.height = h;
	config.width = w;
#ifdef USE_PHY_ADDR
	for(int i = 0; i < CAMERA_BUF_NUM; i++)
		config.phys_addr[i] = CAMERA_BUF_PHY_ADDR_BASE + (i*0x1000000);
#else
	memset(config.phys_addr, 0, sizeof(config.phys_addr));
#endif
	config.log = camera_print_custom;
	
	ret = camera_usb_open(dev_name, &config);
	if(ret){
		printf("open %s failed: %d\n", dev_name, ret);
		goto err;
	}
	printf("camera open success\n");

	ret = camera_usb_start();
	if(ret){
		printf("camera start failed\n");
		goto dev_err;
	}
	printf("camera start success\n");

	//lyp，拍摄n帧图像
	for(index = 0; index < n; index++){
		ret = camera_usb_get_outbuf(&out_buf);
		if(ret){
			printf("camera get buf failed\n");
			goto dev_err;
		}
		printf("camera get buf %d:\n", out_buf.index);

		//lyp，保存图片
		memset(file_name, 0, sizeof(file_name));
		snprintf(file_name, sizeof(file_name), "frame%d_%ux%u.%s", index, w, h, fmt==0?"yuyv":"jpeg");	
		fp = fopen(file_name, "wb");
		if (!fp) {
			printf("open file %s fail\n", file_name);
			goto dev_err;
		}
		fwrite(out_buf.virt_addr, 1, out_buf.len, fp);

		//lyp，归还buf
		ret = camera_usb_prepare_outbuf(&out_buf);
		if(ret){
			printf("camera prepare buf failed\n");
			goto dev_err;
		}
		printf("camera prepare buf %d:\n", out_buf.index);
	}

	ret = camera_usb_stop();
	if(ret){
		printf("camera stop failed: %d\n", ret);
	}
	printf("camera stop success\n");

	ret = camera_usb_close();
	if(ret){
		printf("close %s failed: %d\n", dev_name, ret);
	}
	printf("camera close success\n");

	return ret;

dev_err:
	camera_usb_stop();
	camera_usb_close();
err:
	return ret;
}

