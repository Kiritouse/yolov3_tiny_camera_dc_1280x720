/*
 * DC8000 test source file
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
#include <unistd.h>
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
#include <signal.h>
#include "camera_cap_usb.h"
#include "vc8000d_jpeg.h"
#include "dc8000.h"
#include "yolov3_tiny.h"
#include "opencv_utils.h"
#include "fifo.h"
#include "base-type.h"


#define CAMERA_BUF_NUM 4
#define JPEGDEC_OUTBUF_NUM	4
#define JPEGDEC_INBUF_NUM	4

//lyp，启用ai识别
#define YOLOV3_TINY_ENABLE

//lyp，启用vip前处理缩放功能，然后将只使用一路解码；
//lyp，如果不启用，将使用双路解码，此时其中一路解码将会同时做缩放处理
#define YOLOV3_TINY_PREPROCESS_ENABLE

//lyp，启动该宏后将会显示关于ai的其他信息，前提是使能了宏YOLOV3_ENABLE
#define DISPLAY_AI_INFO
#define AI_MODEL_NAME	"Standard_Yolov3tiny"
#define NETWORK_SIZE	"640x640"

#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...)	printf(fmt, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...)		\
({									\
	if(0)							\
		printf(fmt, ##__VA_ARGS__);	\
})								
#endif

typedef struct{
	camera_usb_outbuf_t usb_outbuf;
	int lock;
	pthread_mutex_t mutex;
}camera_outbuf_t;

typedef struct{
	jpegdec_output_t jpegdec_output;
	const void* jpegdec_handle;
	int frame_id;
}jpegdec_output_wrapper_t;

//lyp，限制frame id计数大小
#define FRAME_ID_MAX	64

static jpegdec_output_wrapper_t* output_wrapper[FRAME_ID_MAX] = {NULL}; 

static int log_level;

//lyp，fifo
FifoInst jpegdec_queue, jpegdec_scaled_queue, display_queue;
#ifdef YOLOV3_TINY_ENABLE
#ifdef YOLOV3_TINY_PREPROCESS_ENABLE
FifoInst yolov3_tiny_process_queue;
#endif
#endif

//lyp，线程标志
static bool camera_capture_thread_end = false;
static bool jpegdec_thread_end = false;
static bool jpegdec_scaled_thread_end = false;
static bool display_thread_end = false;
#ifdef YOLOV3_TINY_ENABLE
#ifdef YOLOV3_TINY_PREPROCESS_ENABLE
static bool yolov3_tiny_process_thread_end = false;
#endif
#endif

//lyp，拍摄图像宽高
static uint32_t cap_width;
static uint32_t cap_height;

//lyp，摄像头buf
static camera_outbuf_t *camera_outbuf[CAMERA_BUF_NUM];

//lyp，线程id
pthread_t camera_tid;
pthread_t jpegdec_tid;
pthread_t jpegdec_scaled_tid;
pthread_t display_tid;
#ifdef YOLOV3_TINY_ENABLE
#ifdef YOLOV3_TINY_PREPROCESS_ENABLE
pthread_t yolov3_tiny_process_tid;
#endif
#endif

static uint8_t coco_color_rgb[USER_YOLOV3_TINY_CLASEE][3] = {
{0, 255, 0}
};


static int camera_print_custom(camera_usb_log_t type, const char* msg)
{
	if(type >= log_level)
		printf("[camera] %s\n", msg);
	return 0;
}


static int dc8000_print_custom(dc8000_log_t type, const char* msg)
{
	if(type >= log_level)
		printf("[dc8000] %s\n", msg);
	return 0;
}


static int jpegdec_print_custom(jpegdec_log_t type, const char* msg)
{
	if(type >= log_level)
		printf("[jpegdec] %s\n", msg);
	return 0;
}


#ifdef YOLOV3_TINY_ENABLE
static int yolov3_tiny_print_custom(yolov3_tiny_log_t type, const char* msg)
{
	if(type >= log_level)
		printf("[yolov3_tiny] %s\n", msg);
	return 0;
}
#endif


static int opencv_utils_custom(opencv_utils_log_t type, const char* msg)
{
	if(type >= log_level)
		printf("[opencv_utils] %s\n", msg);
	return 0;
}


//#define BILLION                                 1000000000
//static uint64_t get_perf_count()
//{
//    struct timespec ts;
//
//    clock_gettime(CLOCK_MONOTONIC, &ts);
//
//    return (uint64_t)((uint64_t)ts.tv_nsec + (uint64_t)ts.tv_sec * BILLION);
//}


static int camera_prepare_outbuf_lock(camera_outbuf_t* outbuf)
{
	int ret = 0;
	
	pthread_mutex_lock(&outbuf->mutex);
	DEBUG_PRINT("[%s] outbuf %u lock: %d\n", __func__, outbuf->usb_outbuf.index, outbuf->lock);
	outbuf->lock--;
	if(outbuf->lock == 0){
		ret = camera_usb_prepare_outbuf(&outbuf->usb_outbuf);
		if(ret){
			printf("camera usb prepare buf %d failed\n", outbuf->usb_outbuf.index);
		}	
	}
	pthread_mutex_unlock(&outbuf->mutex);

	return ret;
}


void* camera_capture_thread(void* arg)
{
	int ret;
	camera_usb_outbuf_t outbuf;

	printf("%s create\n", __func__);

	while(1){
		//lyp，检测线程结束
		if(camera_capture_thread_end) break;
		
		//lyp，获取camera jpeg
		ret = camera_usb_get_outbuf(&outbuf);
		if(ret){
			printf("[%s] camera get buf failed, sleep 50ms...\n", __func__);
			usleep(100);
			continue;
		}

		DEBUG_PRINT("get camera buf %d len %d\n", outbuf.index, outbuf.len);
	
		pthread_mutex_lock(&camera_outbuf[outbuf.index]->mutex);
		camera_outbuf[outbuf.index]->usb_outbuf.index = outbuf.index;
		camera_outbuf[outbuf.index]->usb_outbuf.len = outbuf.len;
		camera_outbuf[outbuf.index]->usb_outbuf.virt_addr = outbuf.virt_addr;
		camera_outbuf[outbuf.index]->usb_outbuf.phys_addr = outbuf.phys_addr;
#ifdef YOLOV3_TINY_ENABLE
#ifdef YOLOV3_TINY_PREPROCESS_ENABLE
		camera_outbuf[outbuf.index]->lock += 1;
		pthread_mutex_unlock(&camera_outbuf[outbuf.index]->mutex);
		//lyp，将buf 发送给解码线程1
		FifoPush(jpegdec_queue, (FifoObject)(camera_outbuf[outbuf.index]), FIFO_EXCEPTION_ENABLE);
#else
		camera_outbuf[outbuf.index]->lock += 2;
		pthread_mutex_unlock(&camera_outbuf[outbuf.index]->mutex);
		//lyp，将buf 发送给解码线程1
		FifoPush(jpegdec_queue, (FifoObject)(camera_outbuf[outbuf.index]), FIFO_EXCEPTION_ENABLE);
		//lyp，将buf 发送给解码线程2
		FifoPush(jpegdec_scaled_queue, (FifoObject)(camera_outbuf[outbuf.index]), FIFO_EXCEPTION_ENABLE);
#endif
#else
		camera_outbuf[outbuf.index]->lock += 1;
		pthread_mutex_unlock(&camera_outbuf[outbuf.index]->mutex);
		//lyp，将buf 发送给解码线程1
		FifoPush(jpegdec_queue, (FifoObject)(camera_outbuf[outbuf.index]), FIFO_EXCEPTION_ENABLE);
#endif
		
	}

	printf("%s exit\n", __func__);

	return NULL;
	
}


void* jpegdec_thread(void* arg)
{
	int ret;
	jpegdec_input_t  jpegdec_input = {0};
	jpegdec_output_t jpegdec_output = {0};
	jpegdec_output_wrapper_t *jpegdec_output_wrapper;
	camera_outbuf_t *cam_outbuf;
	const void* jpegdec_handle;
	bool jpeg_dec_success;
#ifdef YOLOV3_TINY_ENABLE
#ifndef YOLOV3_TINY_PREPROCESS_ENABLE
	user_detected_list list;
	point_t pt1, pt2;
	float fx, fy;
	char txt[128];
	float prob;
#endif
#endif
	int frame_id = 0;

	printf("%s create\n", __func__);

	//lyp，配置解码器(不启用缩放)
	jpegdec_config_t dec_config;
	dec_config.width = cap_width;
	dec_config.height = cap_height;
	dec_config.input_buf_num = JPEGDEC_INBUF_NUM;
	dec_config.output_buf_num = JPEGDEC_OUTBUF_NUM;
	dec_config.log = jpegdec_print_custom;
	dec_config.output_fmt = 0;
	dec_config.scaled_width = cap_width;
	dec_config.scaled_height = cap_height;
	dec_config.scale_enabled = 0;

	//lyp，打开jpeg解码器
	jpegdec_handle = jpegdec_open(&dec_config);
	if(jpegdec_handle == NULL){
		printf("[%s] jpegdec open failed\n", __func__);
		return NULL;
	}

	while(1){
		//lyp，重置frame_id大小，因为output_wrapper容量限制了此大小
		if(frame_id >= FRAME_ID_MAX) frame_id = 0;
		
		//lyp，检测线程退出
		if(jpegdec_thread_end) break;
		
		//lyp，获取摄像头buf
		cam_outbuf = NULL;
		ret = FifoPop(jpegdec_queue, (FifoObject*)&cam_outbuf, FIFO_EXCEPTION_ENABLE);
		if(ret != FIFO_OK){
//			printf("[%s] FifoPop jpegdec_queue failed:%d, sleep 100us...\n", __func__, ret);
			usleep(100);
			continue;
		}
		
		//lyp，jpeg解码
		pthread_mutex_lock(&cam_outbuf->mutex);
		jpegdec_input.size = cam_outbuf->usb_outbuf.len;
		jpegdec_input.virtual_addr = (u32*)(cam_outbuf->usb_outbuf.virt_addr);
		jpegdec_input.bus_address = 0;
		pthread_mutex_unlock(&cam_outbuf->mutex);
		ret = jpegdec_decode(jpegdec_handle, &jpegdec_input, &jpegdec_output);
		if(ret == 0){
			jpeg_dec_success = true;
			DEBUG_PRINT("[%s] jpegdec_decode buf %d success\n", __func__, cam_outbuf->usb_outbuf.index);
		}else{
			jpeg_dec_success = false;
			//lyp，去掉编译警告用
			if(!jpeg_dec_success){
				DEBUG_PRINT("[%s] jpegdec_decode buf %d err: %d\n", __func__, cam_outbuf->usb_outbuf.index, ret);
			}
			//lyp，归还camera buf
			ret = camera_prepare_outbuf_lock(cam_outbuf);
			if(ret){
				printf("[%s] camera_prepare_outbuf_lock failed\n", __func__);
			}
			continue;
		}

#ifdef YOLOV3_TINY_ENABLE
#ifdef YOLOV3_TINY_PREPROCESS_ENABLE
		//lyp，将解码图片同时发送给yolov3-tiny推理线程以及显示线程
		jpegdec_output_wrapper = (jpegdec_output_wrapper_t*)malloc(sizeof(jpegdec_output_wrapper_t));
		jpegdec_output_wrapper->jpegdec_output.virtual_addr = jpegdec_output.virtual_addr;
		jpegdec_output_wrapper->jpegdec_output.bus_address = jpegdec_output.bus_address;
		jpegdec_output_wrapper->jpegdec_output.size = jpegdec_output.size;
		jpegdec_output_wrapper->jpegdec_output.width = jpegdec_output.width;
		jpegdec_output_wrapper->jpegdec_output.height = jpegdec_output.height;
		jpegdec_output_wrapper->jpegdec_output.stride = jpegdec_output.stride;
		jpegdec_output_wrapper->jpegdec_output.stride_ch = jpegdec_output.stride_ch;
		jpegdec_output_wrapper->jpegdec_handle = jpegdec_handle;
		jpegdec_output_wrapper->frame_id = frame_id++;
		FifoPush(yolov3_tiny_process_queue, (FifoObject)(jpegdec_output_wrapper), FIFO_EXCEPTION_ENABLE);
		FifoPush(display_queue, (FifoObject)(jpegdec_output_wrapper), FIFO_EXCEPTION_ENABLE);
#else
		//lyp，获取yolov3结果
		for(;;){
			if(jpegdec_thread_end) break;
			ret = yolov3_tiny_get_result_async(&list);
			if(ret == 0){
				DEBUG_PRINT("[%s] frame %d get result\n", __func__, list.frame_id);
				break;
			}else{
				usleep(100);
			}
		}
		//lyp，打印检测结果
		for(int i = 0; i < list.detected_class_num; i++){
			DEBUG_PRINT("frame %d detect %s(%d) : left_top_x(%d) left_top_y(%d) right_bot_x(%d) right_bot_y(%d) prob(%f) in resolution %ux%u \n", 
				list.frame_id, list.obj[i].name, list.obj[i].name_id, list.obj[i].left_top_x, list.obj[i].left_top_y, 
				list.obj[i].right_bot_x, list.obj[i].right_bot_y, list.obj[i].prob,
				list.image_scaled_width, list.image_scaled_height);
		}

		//lyp，若id匹配，同时解码成功，绘图并发送buf给显示线程
		if((cam_outbuf->usb_outbuf.index == (uint32_t)list.frame_id) && jpeg_dec_success){
			DEBUG_PRINT("[%s] camera buf id(%d) == list id(%d) and jpegdec success\n", __func__, cam_outbuf->usb_outbuf.index, list.frame_id);
			for(int i = 0; i< list.detected_class_num; i++){
				//lyp，计算左上角坐标
				fx = (float)list.obj[i].left_top_x * ((float)jpegdec_output.width / (float)list.image_scaled_width);
				pt1.x = (int)fx;
				fy = (float)list.obj[i].left_top_y * ((float)jpegdec_output.height / (float)list.image_scaled_height);
				pt1.y = (int)fy;
				//lyp，计算右下角坐标
				fx = (float)list.obj[i].right_bot_x * ((float)jpegdec_output.width / (float)list.image_scaled_width);
				pt2.x = (int)fx;
				fy = (float)list.obj[i].right_bot_y * ((float)jpegdec_output.height / (float)list.image_scaled_height);
				pt2.y = (int)fy;
				//lyp，文字
				memset(txt, 0, sizeof(txt));
				prob = list.obj[i].prob * 100;
				snprintf(txt, sizeof(txt), "%s: %.1f%%", list.obj[i].name, prob);
				//lyp，绘图
				opencv_point_object_nv12((uint8_t*)(jpegdec_output.virtual_addr), 
					jpegdec_output.width, jpegdec_output.height, 
					coco_color_rgb[list.obj[i].name_id][0], coco_color_rgb[list.obj[i].name_id][1], coco_color_rgb[list.obj[i].name_id][2],
					pt1, pt2, (const char*)txt);
			}
			//lyp，绘制其他信息
#ifdef DISPLAY_AI_INFO
			pt1.x = 10;
			pt1.y = 50;
			float r = 128;
			float g = 0;
			float b = 128;
			int graph_num;

			memset(txt, 0, sizeof(txt));
			snprintf(txt, sizeof(txt), "AI Model: %s", AI_MODEL_NAME);
			opencv_add_txt_nv12((uint8_t*)(jpegdec_output.virtual_addr),
						 jpegdec_output.width, jpegdec_output.height,
						 r, g, b,
						 pt1, (const char*)txt);

			pt1.y = 100;
			memset(txt, 0, sizeof(txt));
			snprintf(txt, sizeof(txt), "NetworkSize: %s", NETWORK_SIZE);
			opencv_add_txt_nv12((uint8_t*)(jpegdec_output.virtual_addr),
						 jpegdec_output.width, jpegdec_output.height,
						 r, g, b,
						 pt1, (const char*)txt);

			int fps = camera_usb_get_fps();
			pt1.y = 150;
			memset(txt, 0, sizeof(txt));
			snprintf(txt, sizeof(txt), "VideoSize: %dx%d@%dfps", jpegdec_output.width, jpegdec_output.height, fps);
			opencv_add_txt_nv12((uint8_t*)(jpegdec_output.virtual_addr),
						 jpegdec_output.width, jpegdec_output.height,
						 r, g, b,
						 pt1, (const char*)txt);

			graph_num = get_process_graph_num();
			for(int i = 0; i < graph_num; i++){
				pt1.y = 200 + i*50;
				memset(txt, 0, sizeof(txt));
				snprintf(txt, sizeof(txt), "inference time graph%d: %2.2f ms (%2.2f fps)", 
							i, get_process_graph_time_ms(i), 1000/get_process_graph_time_ms(i));
				opencv_add_txt_nv12((uint8_t*)(jpegdec_output.virtual_addr),
							 jpegdec_output.width, jpegdec_output.height,
							 r, g, b,
							 pt1, (const char*)txt);
			}
#endif
			//lyp，将jpegdec_output发送给display线程
			jpegdec_output_wrapper = (jpegdec_output_wrapper_t*)malloc(sizeof(jpegdec_output_wrapper_t));
			jpegdec_output_wrapper->jpegdec_output.virtual_addr = jpegdec_output.virtual_addr;
			jpegdec_output_wrapper->jpegdec_output.bus_address = jpegdec_output.bus_address;
			jpegdec_output_wrapper->jpegdec_output.size = jpegdec_output.size;
			jpegdec_output_wrapper->jpegdec_output.width = jpegdec_output.width;
			jpegdec_output_wrapper->jpegdec_output.height = jpegdec_output.height;
			jpegdec_output_wrapper->jpegdec_output.stride = jpegdec_output.stride;
			jpegdec_output_wrapper->jpegdec_output.stride_ch = jpegdec_output.stride_ch;
			jpegdec_output_wrapper->jpegdec_handle = jpegdec_handle;
			jpegdec_output_wrapper->frame_id = frame_id++;
			FifoPush(display_queue, (FifoObject)(jpegdec_output_wrapper), FIFO_EXCEPTION_ENABLE);
		}
		else if((cam_outbuf->usb_outbuf.index != (uint32_t)list.frame_id) && jpeg_dec_success){
			printf("[%s] camera buf id(%d) != list id(%d), but jpegdec success\n", __func__, cam_outbuf->usb_outbuf.index, list.frame_id);
			//lyp，若id不匹配，但解码成功，发送buf给显示线程
			//lyp，绘制其他信息
#ifdef DISPLAY_AI_INFO
			pt1.x = 10;
			pt1.y = 50;
			float r = 128;
			float g = 0;
			float b = 128;
			int graph_num;

			memset(txt, 0, sizeof(txt));
			snprintf(txt, sizeof(txt), "AI Model: %s", AI_MODEL_NAME);
			opencv_add_txt_nv12((uint8_t*)(jpegdec_output.virtual_addr),
						 jpegdec_output.width, jpegdec_output.height,
						 r, g, b,
						 pt1, (const char*)txt);

			pt1.y = 100;
			memset(txt, 0, sizeof(txt));
			snprintf(txt, sizeof(txt), "NetworkSize: %s", NETWORK_SIZE);
			opencv_add_txt_nv12((uint8_t*)(jpegdec_output.virtual_addr),
						 jpegdec_output.width, jpegdec_output.height,
						 r, g, b,
						 pt1, (const char*)txt);

			int fps = camera_usb_get_fps();
			pt1.y = 150;
			memset(txt, 0, sizeof(txt));
			snprintf(txt, sizeof(txt), "VideoSize: %dx%d@%dfps", jpegdec_output.width, jpegdec_output.height, fps);
			opencv_add_txt_nv12((uint8_t*)(jpegdec_output.virtual_addr),
						 jpegdec_output.width, jpegdec_output.height,
						 r, g, b,
						 pt1, (const char*)txt);

			graph_num = get_process_graph_num();
			for(int i = 0; i < graph_num; i++){
				pt1.y = 200 + i*50;
				memset(txt, 0, sizeof(txt));
				snprintf(txt, sizeof(txt), "inference time graph%d: %2.2f ms (%2.2f fps)", 
							i, get_process_graph_time_ms(i), 1000/get_process_graph_time_ms(i));
				opencv_add_txt_nv12((uint8_t*)(jpegdec_output.virtual_addr),
							 jpegdec_output.width, jpegdec_output.height,
							 r, g, b,
							 pt1, (const char*)txt);
			}
#endif
			jpegdec_output_wrapper = (jpegdec_output_wrapper_t*)malloc(sizeof(jpegdec_output_wrapper_t));
			jpegdec_output_wrapper->jpegdec_output.virtual_addr = jpegdec_output.virtual_addr;
			jpegdec_output_wrapper->jpegdec_output.bus_address = jpegdec_output.bus_address;
			jpegdec_output_wrapper->jpegdec_output.size = jpegdec_output.size;
			jpegdec_output_wrapper->jpegdec_output.width = jpegdec_output.width;
			jpegdec_output_wrapper->jpegdec_output.height = jpegdec_output.height;
			jpegdec_output_wrapper->jpegdec_output.stride = jpegdec_output.stride;
			jpegdec_output_wrapper->jpegdec_output.stride_ch = jpegdec_output.stride_ch;
			jpegdec_output_wrapper->jpegdec_handle = jpegdec_handle;
			jpegdec_output_wrapper->frame_id = frame_id++;
			FifoPush(display_queue, (FifoObject)(jpegdec_output_wrapper), FIFO_EXCEPTION_ENABLE);		
		}else if((cam_outbuf->usb_outbuf.index == (uint32_t)list.frame_id) && !jpeg_dec_success){
			//lyp，若id匹配，但解码失败
			printf("[%s] camera buf id(%d) == list id(%d) but jpegdec failed\n", __func__, cam_outbuf->usb_outbuf.index, list.frame_id);
		}else if((cam_outbuf->usb_outbuf.index != (uint32_t)list.frame_id) && !jpeg_dec_success){
			//lyp，若id不匹配，解码也失败
			printf("[%s] camera buf id(%d) != list id(%d) and jpegdec failed\n", __func__, cam_outbuf->usb_outbuf.index, list.frame_id);
		}
#endif

#else
		jpegdec_output_wrapper = (jpegdec_output_wrapper_t*)malloc(sizeof(jpegdec_output_wrapper_t));
		jpegdec_output_wrapper->jpegdec_output.virtual_addr = jpegdec_output.virtual_addr;
		jpegdec_output_wrapper->jpegdec_output.bus_address = jpegdec_output.bus_address;
		jpegdec_output_wrapper->jpegdec_output.size = jpegdec_output.size;
		jpegdec_output_wrapper->jpegdec_output.width = jpegdec_output.width;
		jpegdec_output_wrapper->jpegdec_output.height = jpegdec_output.height;
		jpegdec_output_wrapper->jpegdec_output.stride = jpegdec_output.stride;
		jpegdec_output_wrapper->jpegdec_output.stride_ch = jpegdec_output.stride_ch;
		jpegdec_output_wrapper->jpegdec_handle = jpegdec_handle;
		jpegdec_output_wrapper->frame_id = frame_id++;
		FifoPush(display_queue, (FifoObject)(jpegdec_output_wrapper), FIFO_EXCEPTION_ENABLE);
#endif

		//lyp，用完后返还camera buf
		ret = camera_prepare_outbuf_lock(cam_outbuf);
		if(ret){
			printf("[%s] camera_prepare_outbuf_lock failed\n", __func__);
		}
		
		//lyp，挂起线程
		usleep(100);
	}

	printf("%s exit\n", __func__);

	//lyp，关闭解码器
	jpegdec_close(jpegdec_handle);
	return NULL;

}


void* jpegdec_scaled_thread(void *arg)
{
	int ret;
	jpegdec_input_t  jpegdec_input = {0};
	jpegdec_output_t jpegdec_output = {0};
	camera_outbuf_t *cam_outbuf;
	const void* jpegdec_handle;

	printf("%s create\n", __func__);

	//lyp，配置解码器(启用缩放，大小为yolov3tiny大小)
	jpegdec_config_t dec_config;
	dec_config.width = cap_width;
	dec_config.height = cap_height;
	dec_config.input_buf_num = JPEGDEC_INBUF_NUM;
	dec_config.output_buf_num = JPEGDEC_OUTBUF_NUM;
	dec_config.log = jpegdec_print_custom;
	dec_config.output_fmt = 0;
	//lyp，yolov3-tiny图片处理大小
	dec_config.scaled_width = 640;
	dec_config.scaled_height = 640;
	dec_config.scale_enabled = 1;

	//lyp，打开jpeg解码器
	jpegdec_handle = jpegdec_open(&dec_config);
	if(jpegdec_handle == NULL){
		printf("[%s] jpegdec open failed\n", __func__);
		return NULL;
	}

	while(1){
		//lyp，检测线程退出
		if(jpegdec_scaled_thread_end) break;
		
		//lyp，获取摄像头buf
		cam_outbuf = NULL;
		ret = FifoPop(jpegdec_scaled_queue, (FifoObject*)&cam_outbuf, FIFO_EXCEPTION_ENABLE);
		if(ret != FIFO_OK){
//			printf("[%s] FifoPop jpegdec_queue failed:%d, sleep 100us...\n", __func__, ret);
			usleep(100);
			continue;
		}
		
		//lyp，jpeg解码
		pthread_mutex_lock(&cam_outbuf->mutex);
		jpegdec_input.size = cam_outbuf->usb_outbuf.len;
		jpegdec_input.virtual_addr = (u32*)(cam_outbuf->usb_outbuf.virt_addr);
		jpegdec_input.bus_address = 0;
		pthread_mutex_unlock(&cam_outbuf->mutex);
		ret = jpegdec_decode(jpegdec_handle, &jpegdec_input, &jpegdec_output);
		if(ret == 0){
			DEBUG_PRINT("[%s] jpegdec_decode buf %d success\n", __func__, cam_outbuf->usb_outbuf.index);
			//lyp，解码成功后才执行yolov3-tiny推理
			ret = yolov3_tiny_process((uint8_t*)jpegdec_output.virtual_addr, cam_outbuf->usb_outbuf.index);
			if(ret){
				printf("[%s] yolov3_tiny_process frame %d failed: %d\n", 
					__func__, cam_outbuf->usb_outbuf.index, ret);
			}
			
			//lyp，用完后归还jpegdec_output
			queue_output_buf(jpegdec_handle, jpegdec_output);
		}else{
			printf("[%s] jpegdec_decode buf %d err: %d\n", __func__, cam_outbuf->usb_outbuf.index, ret);
		}
		
		//lyp，用完后返还camera buf
		ret = camera_prepare_outbuf_lock(cam_outbuf);
		if(ret){
			printf("[%s] camera_prepare_outbuf_lock failed\n", __func__);
		}

		//lyp，挂起线程
		usleep(100);
	}

	printf("%s exit\n", __func__);

	//lyp，关闭解码器
	jpegdec_close(jpegdec_handle);
	return NULL;	
}


#ifdef YOLOV3_TINY_ENABLE
#ifdef YOLOV3_TINY_PREPROCESS_ENABLE
void* yolov3_tiny_process_thread(void* arg)
{
	jpegdec_output_wrapper_t *jpegdec_output_wrapper;
	int ret;

	printf("%s create\n", __func__);

	while(1){
		//lyp，检测线程退出
		if(yolov3_tiny_process_thread_end) break;
		
		//lyp，获取解码图像
		jpegdec_output_wrapper = NULL;
		ret = FifoPop(yolov3_tiny_process_queue, (FifoObject*)&jpegdec_output_wrapper, FIFO_EXCEPTION_ENABLE);
		if(ret != FIFO_OK){
////			printf("[%s] FifoPop yolov3_tiny_process_queue failed:%d, sleep 100us...\n", __func__, ret);
			usleep(100);
			continue;
		}

		//lyp，执行yolov3-tiny推理
		DEBUG_PRINT("[%s] get jpegdec output id(%d), phy(0x%08x)\n", __func__, 
			jpegdec_output_wrapper->frame_id, jpegdec_output_wrapper->jpegdec_output.bus_address);
		ret = yolov3_tiny_process((uint8_t*)jpegdec_output_wrapper->jpegdec_output.virtual_addr, 
			jpegdec_output_wrapper->frame_id);
		if(ret){
			printf("[%s] yolov3_tiny_process frame %d failed: %d\n", 
				__func__, jpegdec_output_wrapper->frame_id, ret);
		}
		
		//lyp，不要归还jpegdec_output，由显示线程归还
//		queue_output_buf(jpegdec_output_wrapper->jpegdec_handle, jpegdec_output_wrapper->jpegdec_output);

		//lyp，不要释放jpegdec_output，由显示线程释放
//		free(jpegdec_output_wrapper);

		//lyp，挂起线程
		usleep(100);
	}

	printf("%s exit\n", __func__);

	return NULL;

}
#endif
#endif


void* display_thread(void* arg)
{
	jpegdec_output_wrapper_t *jpegdec_output_wrapper;
	jpegdec_output_wrapper_t *jpegdec_output_wrapper_free;
	int ret;
#ifdef YOLOV3_TINY_ENABLE
#ifdef YOLOV3_TINY_PREPROCESS_ENABLE
	user_detected_list list;
	point_t pt1, pt2;
	float fx, fy;
	char txt[128];
	float prob;
#endif
#endif
//	int frame_cnt = 0;
	int frame_id;

	printf("%s create\n", __func__);
	
	while(1){
		//lyp，检测线程退出
		if(display_thread_end) break;

		//lyp，获取已经显示完的jpegdec_output_wrapper id
		//lyp，这一步必须放在FifoPop前，防止死锁
		frame_id = dc8000_get_display_done_addr_id();
//		printf("[%s] dc8000_get_display_done_id: %d\n", __func__, frame_id);
		if(frame_id >= 0){
			//lyp，找到对应的jpegdec_output_wrapper
			jpegdec_output_wrapper_free = output_wrapper[frame_id];

			//lyp，归还对应的jpegdec_output
			queue_output_buf(jpegdec_output_wrapper_free->jpegdec_handle, jpegdec_output_wrapper_free->jpegdec_output);

			//lyp，释放jpegdec_output_wrapper
			free(jpegdec_output_wrapper_free);
			jpegdec_output_wrapper_free = NULL;
		}else{
			DEBUG_PRINT("[%s] dc8000_get_display_done_id failed\n", __func__);
		}

		//lyp，获取解码图像
		jpegdec_output_wrapper = NULL;
		ret = FifoPop(display_queue, (FifoObject*)&jpegdec_output_wrapper, FIFO_EXCEPTION_ENABLE);
		if(ret != FIFO_OK){
			usleep(100);
			continue;
		}

		DEBUG_PRINT("[%s] FifoPop display_queue success: phy(0x%08x)\n", __func__, jpegdec_output_wrapper->jpegdec_output.bus_address);

#ifdef YOLOV3_TINY_ENABLE
#ifdef YOLOV3_TINY_PREPROCESS_ENABLE
		//lyp，等待yolov3 tiny处理结果
		for(;;){
			if(display_thread_end) break;
			ret = yolov3_tiny_get_result_async(&list);
			if(ret == 0){
				DEBUG_PRINT("[%s] frame %d get result\n", __func__, list.frame_id);
				break;
			}else{
				usleep(100);
			}
		}
		//lyp，打印检测结果
		for(int i = 0; i < list.detected_class_num; i++){
			DEBUG_PRINT("[%s] frame %d detect %s(%d) : left_top_x(%d) left_top_y(%d) right_bot_x(%d) right_bot_y(%d) prob(%f) in resolution %ux%u \n", 
				__func__, list.frame_id, list.obj[i].name, list.obj[i].name_id, list.obj[i].left_top_x, list.obj[i].left_top_y, 
				list.obj[i].right_bot_x, list.obj[i].right_bot_y, list.obj[i].prob,
				list.image_scaled_width, list.image_scaled_height);
		}

		//lyp，若id匹配，绘图并显示图像
		if(jpegdec_output_wrapper->frame_id == list.frame_id){
			DEBUG_PRINT("[%s] jpegdec frame id(%d) == list id(%d)\n", __func__, jpegdec_output_wrapper->frame_id, list.frame_id);
			//lyp，绘图
			for(int i = 0; i< list.detected_class_num; i++){
				//lyp，计算左上角坐标
				fx = (float)list.obj[i].left_top_x * ((float)jpegdec_output_wrapper->jpegdec_output.width / (float)list.image_scaled_width);
				pt1.x = (int)fx;
				fy = (float)list.obj[i].left_top_y * ((float)jpegdec_output_wrapper->jpegdec_output.height / (float)list.image_scaled_height);
				pt1.y = (int)fy;
				//lyp，计算右下角坐标
				fx = (float)list.obj[i].right_bot_x * ((float)jpegdec_output_wrapper->jpegdec_output.width / (float)list.image_scaled_width);
				pt2.x = (int)fx;
				fy = (float)list.obj[i].right_bot_y * ((float)jpegdec_output_wrapper->jpegdec_output.height / (float)list.image_scaled_height);
				pt2.y = (int)fy;
				//lyp，文字
				memset(txt, 0, sizeof(txt));
				prob = list.obj[i].prob * 100;
				snprintf(txt, sizeof(txt), "%s: %.1f%%", list.obj[i].name, prob);
				//lyp，绘图，绿色
				opencv_point_object_nv12((uint8_t*)(jpegdec_output_wrapper->jpegdec_output.virtual_addr), 
					jpegdec_output_wrapper->jpegdec_output.width, jpegdec_output_wrapper->jpegdec_output.height, 
					coco_color_rgb[list.obj[i].name_id][0], coco_color_rgb[list.obj[i].name_id][1], coco_color_rgb[list.obj[i].name_id][2],
					pt1, pt2, (const char*)txt);
			}
		}
		//lyp，若id不匹配，只显示图像，不绘图
		else{
			//printf("[%s] jpegdec frame id(%d) != list id(%d)\n", __func__, jpegdec_output_wrapper->frame_id, list.frame_id);
		}

		//lyp，显示其他信息
#ifdef DISPLAY_AI_INFO
		pt1.x = 10;
		pt1.y = 50;
		float r = 128;
		float g = 0;
		float b = 128;
		int graph_num;

		memset(txt, 0, sizeof(txt));
		snprintf(txt, sizeof(txt), "AI Model: %s", AI_MODEL_NAME);
		opencv_add_txt_nv12((uint8_t*)(jpegdec_output_wrapper->jpegdec_output.virtual_addr),
					 jpegdec_output_wrapper->jpegdec_output.width, jpegdec_output_wrapper->jpegdec_output.height,
					 r, g, b,
					 pt1, (const char*)txt);

		pt1.y = 100;
		memset(txt, 0, sizeof(txt));
		snprintf(txt, sizeof(txt), "NetworkSize: %s", NETWORK_SIZE);
		opencv_add_txt_nv12((uint8_t*)(jpegdec_output_wrapper->jpegdec_output.virtual_addr),
					 jpegdec_output_wrapper->jpegdec_output.width, jpegdec_output_wrapper->jpegdec_output.height,
					 r, g, b,
					 pt1, (const char*)txt);

		int fps = camera_usb_get_fps();
		pt1.y = 150;
		memset(txt, 0, sizeof(txt));
		snprintf(txt, sizeof(txt), "VideoSize: %dx%d@%dfps", 
			jpegdec_output_wrapper->jpegdec_output.width, jpegdec_output_wrapper->jpegdec_output.height, fps);
		opencv_add_txt_nv12((uint8_t*)(jpegdec_output_wrapper->jpegdec_output.virtual_addr),
					 jpegdec_output_wrapper->jpegdec_output.width, jpegdec_output_wrapper->jpegdec_output.height,
					 r, g, b,
					 pt1, (const char*)txt);

		graph_num = get_process_graph_num();
		for(int i = 0; i < graph_num; i++){
			pt1.y = 200 + i*50;
			memset(txt, 0, sizeof(txt));
			snprintf(txt, sizeof(txt), "inference time graph%d: %2.2f ms (%2.2f fps)", 
						i, get_process_graph_time_ms(i), 1000/get_process_graph_time_ms(i));
			opencv_add_txt_nv12((uint8_t*)(jpegdec_output_wrapper->jpegdec_output.virtual_addr),
						 jpegdec_output_wrapper->jpegdec_output.width, jpegdec_output_wrapper->jpegdec_output.height,
						 r, g, b,
						 pt1, (const char*)txt);
		}
#endif

#endif
#endif

		//lyp，显示jpeg解码后图像
		dc8000_display_async(jpegdec_output_wrapper->jpegdec_output.bus_address , 
			jpegdec_output_wrapper->jpegdec_output.bus_address + \
			(jpegdec_output_wrapper->jpegdec_output.width*jpegdec_output_wrapper->jpegdec_output.height), 
			jpegdec_output_wrapper->frame_id);

		//lyp，保存jpegdec_output_wrapper
		output_wrapper[jpegdec_output_wrapper->frame_id] = jpegdec_output_wrapper;


		//lyp，挂起线程
		usleep(100);
	}

	printf("%s exit\n", __func__);

	return NULL;		
}


static void usage(void)
{
	printf("Usage: yolov3-tiny test [option] ..\n");
	printf("\t -d camera id \n");
	printf("\t -w camera image width \n");
	printf("\t -h camera image height \n");
	printf("\t -f yolov3 tiny network binary file path \n");
	printf("\t -l log level 0~3 \n");
}


void program_exit(int sig)
{
	jpegdec_output_wrapper_t *jpegdec_output_wrapper;
	int ret;

	printf("capture a signal: %d\n", sig);
	
	//lyp，结束线程
	camera_capture_thread_end = true;
	//lyp，等待所有拍摄到的图片处理完后再停止后面的线程
	sleep(1);
	jpegdec_thread_end = true;
	jpegdec_scaled_thread_end = true;
	display_thread_end = true;
#ifdef YOLOV3_TINY_ENABLE
#ifdef YOLOV3_TINY_PREPROCESS_ENABLE
	yolov3_tiny_process_thread_end = true;
#endif
#endif
	
	//lyp，回收线程
	pthread_join(camera_tid, NULL);
	pthread_join(jpegdec_tid, NULL);
#ifdef YOLOV3_TINY_ENABLE
#ifndef YOLOV3_TINY_PREPROCESS_ENABLE
	pthread_join(jpegdec_scaled_tid, NULL);
#else
	pthread_join(yolov3_tiny_process_tid, NULL);
#endif
#endif
	pthread_join(display_tid, NULL);

	//lyp，清空fifo，并归还jpeg
	while(1){
		jpegdec_output_wrapper = NULL;
		ret = FifoPop(display_queue, (FifoObject*)&jpegdec_output_wrapper, FIFO_EXCEPTION_ENABLE);
		if(ret == FIFO_OK){
			//lyp，归还jpegdec_output
			queue_output_buf(jpegdec_output_wrapper->jpegdec_handle, jpegdec_output_wrapper->jpegdec_output);
			free(jpegdec_output_wrapper);
			continue;
		}
		break;
	}
	
#ifdef YOLOV3_TINY_ENABLE
#ifdef YOLOV3_TINY_PREPROCESS_ENABLE
	//lyp，清空fifo，并归还jpeg
	while(1){
		jpegdec_output_wrapper = NULL;
		ret = FifoPop(yolov3_tiny_process_queue, (FifoObject*)&jpegdec_output_wrapper, FIFO_EXCEPTION_ENABLE);
		if(ret == FIFO_OK){
			//lyp，归还jpegdec_output
			queue_output_buf(jpegdec_output_wrapper->jpegdec_handle, jpegdec_output_wrapper->jpegdec_output);
			free(jpegdec_output_wrapper);
			continue;
		}
		break;
	}
#endif
#endif
	
	//lyp，释放fifo
	FifoRelease(jpegdec_queue);
	FifoRelease(jpegdec_scaled_queue);
	FifoRelease(display_queue);
#ifdef YOLOV3_TINY_ENABLE
#ifdef YOLOV3_TINY_PREPROCESS_ENABLE
	FifoRelease(yolov3_tiny_process_queue);
#endif
#endif

	//lyp，归还并释放camera buf
	for(int i = 0; i < CAMERA_BUF_NUM; i++){
		camera_usb_prepare_outbuf(&camera_outbuf[i]->usb_outbuf);
		pthread_mutex_destroy(&(camera_outbuf[i]->mutex));
		free(camera_outbuf[i]);
	}
	
	//lyp，关闭硬件
	dc8000_close();
	yolov3_tiny_release();
	camera_usb_stop();
	camera_usb_close();

	printf("program exit......\n");

	exit(0);

}


int main(int argc, char *argv[])
{
	uint32_t i;
	int ret = 0;
	int opt;
	char dev_name[32] = "/dev/video";
	yolov3_tiny_config_t yolov3_tiny_config = {0};
	camera_usb_config_t camera_config = {0};
	jpegdec_output_wrapper_t *jpegdec_output_wrapper;

	if(argc < 2){
		usage();
		exit(0);
	}

	while ((opt = getopt(argc, argv, "d:w:h:f:l:")) != -1){
		switch (opt) {
		case 'd':
			strcat(dev_name, optarg);
			break;
		case 'w':
			cap_width = atoi(optarg);
			break;
		case 'h':
			cap_height = atoi(optarg);
			break;
		case 'l':
			log_level = atoi(optarg);
			break;
		case 'f':
			strcpy((char*)yolov3_tiny_config.data_name, optarg);
			break;
		default:
			usage();
			return -1;
		}
	}

	//lyp，初始化fifo
  	if (FifoInit(CAMERA_BUF_NUM*2, &(jpegdec_queue)) != FIFO_OK) {
		printf("jpeggdec_queue fifo init failed\n");
    	goto exit;
  	}
  	if (FifoInit(CAMERA_BUF_NUM*2, &(jpegdec_scaled_queue)) != FIFO_OK) {
		printf("jpegdec_scaled_queue fifo init failed\n");
    	goto exit;
  	}
  	if (FifoInit(CAMERA_BUF_NUM*2, &(display_queue)) != FIFO_OK) {
		printf("display_queue fifo init failed\n");
    	goto exit;
  	}
#ifdef YOLOV3_TINY_ENABLE
#ifdef YOLOV3_TINY_PREPROCESS_ENABLE
  	if (FifoInit(CAMERA_BUF_NUM*2, &(yolov3_tiny_process_queue)) != FIFO_OK) {
		printf("display_queue fifo init failed\n");
    	goto exit;
  	}
#endif
#endif

	//lyp，配置摄像头，使用mjpeg格式
	camera_config.buf_num = CAMERA_BUF_NUM;
	camera_config.format = 1;
	camera_config.height = cap_height;
	camera_config.width = cap_width;
	memset(camera_config.phys_addr, 0, sizeof(camera_config.phys_addr));
	camera_config.log = camera_print_custom;
	//lyp，打开摄像头
	ret = camera_usb_open(dev_name, &camera_config);
	if(ret){
		printf("open %s failed: %d\n", dev_name, ret);
		goto exit;
	}
	printf("camera open success\n");
	//lyp，启动摄像头
	ret = camera_usb_start();
	if(ret){
		printf("camera start failed\n");
		goto exit;
	}
	printf("camera start success\n");
	//lyp，申请摄像头缓存
	for(i = 0; i < CAMERA_BUF_NUM; i++){
		camera_outbuf[i] = (camera_outbuf_t*)malloc(sizeof(camera_outbuf_t));
		memset(camera_outbuf[i], 0, sizeof(camera_outbuf_t));
		pthread_mutex_init(&(camera_outbuf[i]->mutex), NULL);
	}
	
	//lyp，打开显示器
	ret = dc8000_open(cap_width, cap_height, dc8000_print_custom);
	if(ret){
		printf("dc8000 open failed: %d\n", ret);
		goto exit;
	}

#ifdef YOLOV3_TINY_ENABLE	
	//lyp，yolov3-tiny-nbg初始化
	yolov3_tiny_config.log = yolov3_tiny_print_custom;
#ifdef YOLOV3_TINY_PREPROCESS_ENABLE
	yolov3_tiny_config.width = cap_width;
	yolov3_tiny_config.height = cap_height;
#else
	//lyp，不启用yolov3-tiny前处理，这里需要设置为416x416
	//lyp，这样yolov3-tiny将不会对图像进行缩放
	yolov3_tiny_config.width = 416;
	yolov3_tiny_config.height = 416;
#endif
	//lyp，nv12
	yolov3_tiny_config.format = 0;
	ret = yolov3_tiny_init(&yolov3_tiny_config);
	if(ret){
		printf("yolov3_tiny_nbg_init failed: %d\n", ret);
		goto exit;
	}
#endif

	//lyp，opencv初始化
	opencv_utils_set_log_func(opencv_utils_custom);

	//lyp，初始化线程标志
	camera_capture_thread_end = false;
	jpegdec_thread_end = false;
	jpegdec_scaled_thread_end = false;
	display_thread_end = false;
#ifdef YOLOV3_TINY_ENABLE
#ifdef YOLOV3_TINY_PREPROCESS_ENABLE
	yolov3_tiny_process_thread_end = false;
#endif
#endif

	//lyp，创建 摄像头 线程
	pthread_create(&camera_tid, NULL, camera_capture_thread, NULL);
	
	//lyp，创建 (解码原图+yolov3接收+画框/解码原图+yolov3发送) 线程
	pthread_create(&jpegdec_tid, NULL, jpegdec_thread, NULL);

#ifdef YOLOV3_TINY_ENABLE
#ifndef YOLOV3_TINY_PREPROCESS_ENABLE
	//lyp，创建 解码416x416+yolov3推理 线程
	pthread_create(&jpegdec_scaled_tid, NULL, jpegdec_scaled_thread, NULL);
#else 
	//lyp，创建 yolov3 tiny推理线程
	pthread_create(&yolov3_tiny_process_tid, NULL, yolov3_tiny_process_thread, NULL);
#endif
#endif

	//lyp，创建 显示 线程
	pthread_create(&display_tid, NULL, display_thread, NULL);

	//lyp，捕获结束信号
	signal(SIGINT, program_exit);
		
	//lyp，回收线程
	pthread_join(camera_tid, NULL);
	pthread_join(jpegdec_tid, NULL);
#ifdef YOLOV3_TINY_ENABLE
#ifndef YOLOV3_TINY_PREPROCESS_ENABLE
	pthread_join(jpegdec_scaled_tid, NULL);
#else
	pthread_join(yolov3_tiny_process_tid, NULL);
#endif
#endif
	pthread_join(display_tid, NULL);

exit:
	//lyp，清空fifo，并归还jpeg
	while(1){
		jpegdec_output_wrapper = NULL;
		ret = FifoPop(display_queue, (FifoObject*)&jpegdec_output_wrapper, FIFO_EXCEPTION_ENABLE);
		if(ret == FIFO_OK){
			//lyp，归还jpegdec_output
			queue_output_buf(jpegdec_output_wrapper->jpegdec_handle, jpegdec_output_wrapper->jpegdec_output);
			free(jpegdec_output_wrapper);
			continue;
		}
		break;
	}

#ifdef YOLOV3_TINY_ENABLE
#ifdef YOLOV3_TINY_PREPROCESS_ENABLE
	//lyp，清空fifo，并归还jpeg
	while(1){
		jpegdec_output_wrapper = NULL;
		ret = FifoPop(yolov3_tiny_process_queue, (FifoObject*)&jpegdec_output_wrapper, FIFO_EXCEPTION_ENABLE);
		if(ret == FIFO_OK){
			//lyp，归还jpegdec_output
			queue_output_buf(jpegdec_output_wrapper->jpegdec_handle, jpegdec_output_wrapper->jpegdec_output);
			free(jpegdec_output_wrapper);
			continue;
		}
		break;
	}
#endif
#endif

	//lyp，释放fifo
	FifoRelease(jpegdec_queue);
	FifoRelease(jpegdec_scaled_queue);
	FifoRelease(display_queue);
#ifdef YOLOV3_TINY_ENABLE
#ifdef YOLOV3_TINY_PREPROCESS_ENABLE
	FifoRelease(yolov3_tiny_process_queue);
#endif
#endif

	//lyp，归还并释放camera buf
	for(i = 0; i < CAMERA_BUF_NUM; i++){
		camera_usb_prepare_outbuf(&camera_outbuf[i]->usb_outbuf);
		pthread_mutex_destroy(&(camera_outbuf[i]->mutex));
		free(camera_outbuf[i]);
	}
	
	//lyp，关闭硬件
	dc8000_close();
#ifdef YOLOV3_TINY_ENABLE
	yolov3_tiny_release();
#endif
	camera_usb_stop();
	camera_usb_close();

	printf("test end......\n");

	exit(0);

}


