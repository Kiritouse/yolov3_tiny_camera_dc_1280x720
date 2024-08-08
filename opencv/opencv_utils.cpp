/*
 * opencv test source file
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
#include <stdarg.h>
#include <iostream> 
#include <opencv2/core/core.hpp> 
#include <opencv2/highgui/highgui.hpp> 
#include <opencv2/opencv.hpp>
#include "opencv_utils.h"


using namespace cv; 
using namespace std;

/* bt.601 下的 GRB 转 yuv */
#define     GET_Y_BT601(R,G,B)  ((uint8_t)(16 +  0.257 * (R) + 0.504 * (G) + 0.098 * (B)))
#define     GET_U_BT601(R,G,B)  ((uint8_t)(128 - 0.148 * (R) - 0.291 * (G) + 0.439 * (B)))
#define     GET_V_BT601(R,G,B)  ((uint8_t)(128 + 0.439 * (R) - 0.368 * (G) - 0.071 * (B)))

/* bt.709 下的 GRB 转 yuv */
#define     GET_Y_BT709(R,G,B)  ((uint8_t)(16   + 0.183 * (R) + 0.614 * (G) + 0.062 * (B)))
#define     GET_U_BT709(R,G,B)  ((uint8_t)(128 - 0.101 * (R) - 0.339 * (G) + 0.439 * (B)))
#define     GET_V_BT709(R,G,B)  ((uint8_t)(128 + 0.439 * (R) - 0.399 * (G) - 0.040 * (B)))

static opencv_utils_log_func_t opencv_utils_log;
static pthread_mutex_t opencv_utils_log_mutex = PTHREAD_MUTEX_INITIALIZER;


static int opencv_utils_print(opencv_utils_log_t log_level, const char *fmt, ...) 
{
	va_list args;
	int i;
	char log_buf[OPENCV_UTILS_LOG_BUF_LEN];
	size_t size = OPENCV_UTILS_LOG_BUF_LEN;

	memset(log_buf, 0, size);

	va_start(args, fmt);
	i = vsnprintf(log_buf, size, fmt, args);
	va_end(args);

	pthread_mutex_lock(&opencv_utils_log_mutex);
	if(opencv_utils_log) opencv_utils_log(log_level, log_buf);
	pthread_mutex_unlock(&opencv_utils_log_mutex);

	return i;	
}


int opencv_utils_set_log_func(opencv_utils_log_func_t log)
{
	opencv_utils_log = log;
	opencv_utils_print(OPENCV_UTILS_INFO, "set log func success\n");
	return 0;
}


int opencv_point_object_nv12(uint8_t* nv12, uint32_t width, uint32_t height, 
									float r, float g, float b,
									point_t pt1, point_t pt2, 
									const char* object_name)

{
	string txt = object_name;

	uint8_t y = GET_Y_BT601(r, g, b);
	uint8_t u = GET_U_BT601(r, g, b);
	uint8_t v = GET_V_BT601(r, g, b);

	cv::Mat nv12_y_Mat = cv::Mat (height, width, CV_8UC1, (void*)nv12);
	cv::rectangle(nv12_y_Mat, cv::Point(pt1.x, pt1.y), cv::Point(pt2.x, pt2.y), cv::Scalar(y), 2, 8, 0);
	cv::putText(nv12_y_Mat, txt, cv::Point(pt1.x, pt1.y - 10), FONT_HERSHEY_SIMPLEX  , 1.5, cv::Scalar(y), 2, 8, false);

	cv::Mat nv12_uv_Mat = cv::Mat (height / 2, width / 2, CV_8UC2, (void*)(nv12 + (width * height)));
	cv::rectangle(nv12_uv_Mat, cv::Point(pt1.x/2, pt1.y/2), cv::Point(pt2.x/2, pt2.y/2), cv::Scalar(u, v), 1, 8, 0);
	cv::putText(nv12_uv_Mat, txt, cv::Point(pt1.x/2, (pt1.y - 10)/2), FONT_HERSHEY_SIMPLEX  , 0.75, cv::Scalar(u, v), 1, 8, false);

	return 0;
}


int opencv_add_txt_nv12(uint8_t* nv12, uint32_t width, uint32_t height, 
                                    float r, float g, float b,
                                    point_t pt, const char* text)
{
    string txt = text;

    uint8_t y = GET_Y_BT601(r, g, b); 
    uint8_t u = GET_U_BT601(r, g, b); 
    uint8_t v = GET_V_BT601(r, g, b); 

	//lyp，初始值为1080p合适值
	double y_font_scale = 1.5;

	//lyp，根据分辨率计算合适值
	y_font_scale = ((double)width / 1920) * 1.5;

    cv::Mat nv12_y_Mat = cv::Mat (height, width, CV_8UC1, (void*)nv12);
    cv::putText(nv12_y_Mat, txt, cv::Point(pt.x, pt.y), FONT_HERSHEY_SIMPLEX  , y_font_scale, cv::Scalar(y), 2, 8, false);

    cv::Mat nv12_uv_Mat = cv::Mat (height / 2, width / 2, CV_8UC2, (void*)(nv12 + (width * height)));
    cv::putText(nv12_uv_Mat, txt, cv::Point(pt.x/2, pt.y/2), FONT_HERSHEY_SIMPLEX  , y_font_scale/2, cv::Scalar(u, v), 1, 8, false);

    return 0;
}


