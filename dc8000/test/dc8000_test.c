/*
 * DC8000 test source file
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
#include <sys/mman.h>
#include "dc8000.h"

static int log_level;

#define DISPLAY_BUFFER_NUM 4
#define DISPLAY_BUFFER_START 0x20000000


#define BILLION                                 1000000000
static uint64_t get_perf_count()
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (uint64_t)((uint64_t)ts.tv_nsec + (uint64_t)ts.tv_sec * BILLION);
}


static int dc8000_print_custom(dc8000_log_t type, const char* msg)
{
	if(type >= log_level)
		printf("[dc8000] %s\n", msg);
}


static void usage(void)
{
	printf("Usage: dc8000 test [option] ..\n");
	printf("\t -i input nv12 file name \n");
	printf("\t -w disply width \n");
	printf("\t -h disply height \n");
	printf("\t -l log level 0~3 \n");
}


int main(int argc, char *argv[])
{

	u32 user_addr;
	FILE *fin = NULL;
	u8 *yuv_data;
	u32 width, height, yuv_size, uv_offset;
	u32 y_addr, uv_addr;
	u8 *display_buffer_user[DISPLAY_BUFFER_NUM];
	u32 display_buffer_phys[DISPLAY_BUFFER_NUM];
	int i;
	int opt;

	if(argc < 2){
		usage();
		exit(0);
	}

	while ((opt = getopt(argc, argv, "i:w:h:l:")) != -1){
		switch (opt) {
		case 'i':
			fin = fopen(optarg, "rb");
			if (!fin) {
				printf("open in file %s fail\n", optarg);
				return -1;
			}
			break;
		case 'w':
			width = atoi(optarg);
			break;
		case 'h':
			height = atoi(optarg);
			break;
		case 'l':
			log_level = atoi(optarg);
			break;
		default:
			usage();
			return -1;
		}
	}

	yuv_size = width * height * 3 / 2;
	uv_offset = width * height;

    //yuv_size = width * height * 4;

 	mmap_image_buffer(&user_addr, DISPLAY_BUFFER_START, yuv_size * DISPLAY_BUFFER_NUM);
	for (i = 0; i < DISPLAY_BUFFER_NUM; i++) {
		display_buffer_user[i] = (u8 *) (user_addr + i * yuv_size);
		display_buffer_phys[i] = DISPLAY_BUFFER_START + i * yuv_size;
		printf("display buffer[%d]: phys_addr = 0x%08x, user_addr = %p\n", i,
				display_buffer_phys[i], display_buffer_user[i]);
	}

	dc8000_open(width, height, dc8000_print_custom);

	i = 0;
	while (1) {
		yuv_data = display_buffer_user[i];
		if (fread(yuv_data, yuv_size, 1, fin) != 1)
			break;

		y_addr = display_buffer_phys[i];
		uv_addr = y_addr + uv_offset;
		dc8000_display_sync(y_addr, uv_addr);

		i = (i + 1) % DISPLAY_BUFFER_NUM;
	}

	fclose(fin);
	munmap_image_buffer(&user_addr, yuv_size * DISPLAY_BUFFER_NUM);

	dc8000_close();
	printf("display done\n");
	return 0;

}


