/*
 * yolov3 tiny test source file
 *
 * Maintainer: Yiping.Liang <Yiping.Liang@orbita.com>
 *
 * Copyright (C) 2021 Orbita Inc.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <semaphore.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include "vsi_nn_pub.h"
#include "vnn_global.h"
#include "vnn_pre_process.h"
#include "vnn_post_process.h"
#include "vnn_yolov3tinyuint8.h"
#include "fifo.h"
#include "yolov3_tiny.h"


#define _BASETSD_H

//lyp，log相关
static yolov3_tiny_log_func_t yolov3_tiny_log;
static pthread_mutex_t yolov3_tiny_log_mutex = PTHREAD_MUTEX_INITIALIZER;

//lyp，fifo
FifoInst process_graph_queue, post_graph_queue, get_result_queue;

static struct Vnn_Wrapper *vnn_wrapper[YOLOV3_TINY_GRAPH_NUM] = {NULL};

//lyp，线程标志
static bool vnn_process_graph_thread_end = false;
static bool vnn_post_graph_thread_end = false;

//lyp，运行耗时
static uint64_t vnn_copy_tensor_ms, vnn_copy_tensor_us;
static uint64_t vnn_process_graph_ms, vnn_process_graph_us;
static double vnn_process_graph_ms_current[YOLOV3_TINY_GRAPH_NUM];
static uint64_t vnn_post_graph_ms, vnn_post_graph_us;
static uint64_t vnn_get_tensor_ms, vnn_get_tensor_us;

//lyp，线程id
pthread_t vnn_progess_graph_tid, vnn_post_graph_tid; 

//lyp，帧数累计
static uint32_t frame_cnt;

//lyp，图像尺寸
static uint32_t width;
static uint32_t height;


static int yolov3_tiny_print(yolov3_tiny_log_t log_level, const char *fmt, ...) 
{
	va_list args;
	int i;
	char log_buf[YOLOV3_TINY_LOG_BUF_LEN];
	size_t size = YOLOV3_TINY_LOG_BUF_LEN;

	memset(log_buf, 0, size);

	va_start(args, fmt);
	i = vsnprintf(log_buf, size, fmt, args);
	va_end(args);

	pthread_mutex_lock(&yolov3_tiny_log_mutex);
	if(yolov3_tiny_log) yolov3_tiny_log(log_level, log_buf);
	pthread_mutex_unlock(&yolov3_tiny_log_mutex);

	return i;	
}


static void vnn_ReleaseNeuralNetwork
    (
    vsi_nn_graph_t *graph
    )
{
    vnn_ReleaseYolov3TinyUint8( graph, TRUE );
    if (vnn_UseImagePreprocessNode())
    {
        vnn_ReleaseBufferImage();
    }
}

static vsi_status vnn_PostProcessNeuralNetwork
    (
    struct Vnn_Wrapper *vnn_wrapper,
    detected_list* list
    )
{
    return vnn_PostProcessYolov3TinyUint8( vnn_wrapper, list );
}


#define BILLION                                 1000000000
static uint64_t get_perf_count()
{
#if defined(__linux__) || defined(__ANDROID__) || defined(__QNX__) || defined(__CYGWIN__)
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (uint64_t)((uint64_t)ts.tv_nsec + (uint64_t)ts.tv_sec * BILLION);
#elif defined(_WIN32) || defined(UNDER_CE)
    LARGE_INTEGER ln;

    QueryPerformanceCounter(&ln);

    return (uint64_t)ln.QuadPart;
#endif
}


static vsi_status vnn_VerifyGraph
    (
    vsi_nn_graph_t *graph
    )
{
    vsi_status status = VSI_FAILURE;
    uint64_t tmsStart, tmsEnd, msVal, usVal;

    /* Verify graph */
    yolov3_tiny_print(YOLOV3_TINY_INFO, "Verify...\n");
    tmsStart = get_perf_count();
    status = vsi_nn_VerifyGraph( graph );
    TEST_CHECK_STATUS(status, final);
    tmsEnd = get_perf_count();
    msVal = (tmsEnd - tmsStart)/1000000;
    usVal = (tmsEnd - tmsStart)/1000;
    yolov3_tiny_print(YOLOV3_TINY_INFO, "Verify Graph: %llums or %lluus\n", msVal, usVal);

final:
    return status;
}


#define VNN_APP_ASYNC_RUN
static vsi_status vnn_ProcessGraph
    (
    vsi_nn_graph_t *graph
    )
{
    vsi_status status = VSI_FAILURE;
    int32_t i,loop;
    char *loop_s;
//    uint64_t tmsStart, tmsEnd, sigStart, sigEnd;
//    float msVal, usVal;

    status = VSI_FAILURE;
    loop = 1; /* default loop time is 1 */
    loop_s = getenv("VNN_LOOP_TIME");
    if(loop_s)
    {
        loop = atoi(loop_s);
    }

    /* Run graph */
//    tmsStart = get_perf_count();
    yolov3_tiny_print(YOLOV3_TINY_DEBUG, "Start run graph [%d] times...\n", loop);
    for(i = 0; i < loop; i++)
    {
//        sigStart = get_perf_count();
#ifdef VNN_APP_ASYNC_RUN
        status = vsi_nn_AsyncRunGraph( graph );
        if(status != VSI_SUCCESS)
        {
            yolov3_tiny_print(YOLOV3_TINY_ERR, "Async Run graph the %d time fail\n", i);
        }
        TEST_CHECK_STATUS( status, final );

        //do something here...
        yolov3_tiny_print(YOLOV3_TINY_DEBUG, "async run graph......\n");

        status = vsi_nn_AsyncRunWait( graph );
        if(status != VSI_SUCCESS)
        {
            yolov3_tiny_print(YOLOV3_TINY_ERR, "Wait graph the %d time fail\n", i);
        }
#else
        status = vsi_nn_RunGraph( graph );
        if(status != VSI_SUCCESS)
        {
            yolov3_tiny_print(YOLOV3_TINY_ERR, "Run graph the %d time fail\n", i);
        }
#endif
        TEST_CHECK_STATUS( status, final );

//        sigEnd = get_perf_count();
//        msVal = (sigEnd - sigStart)/1000000;
//        usVal = (sigEnd - sigStart)/1000;
//        yolov3_tiny_print(YOLOV3_TINY_DEBUG, "Run the %u time: %.2fms or %.2fus\n", (i + 1), msVal, usVal);
    }
//    tmsEnd = get_perf_count();
//    msVal = (tmsEnd - tmsStart)/1000000;
//    usVal = (tmsEnd - tmsStart)/1000;
//    yolov3_tiny_print(YOLOV3_TINY_DEBUG, "vxProcessGraph execution time:\n");
//    yolov3_tiny_print(YOLOV3_TINY_DEBUG, "Total   %.2fms or %.2fus\n", msVal, usVal);
//    yolov3_tiny_print(YOLOV3_TINY_DEBUG, "Average %.2fms or %.2fus\n", ((float)usVal)/1000/loop, ((float)usVal)/loop);

final:
    return status;
}


static vsi_nn_graph_t *vnn_CreateNeuralNetwork
    (
    const char *data_file_name,
    uint8_t * output_data_1,
    uint8_t * output_data_2,
    uint8_t * output_data_3,
    data_file_type data_type,
    uint32_t image_width,
    uint32_t image_height
    )
{
    vsi_nn_graph_t *graph = NULL;
    uint64_t tmsStart, tmsEnd, msVal, usVal;

    //lyp，判断网络文件类型
    tmsStart = get_perf_count();
    if(data_type == FILE_DATA){
		graph = vnn_CreateYolov3TinyUint8_data( data_file_name, NULL,
						  vnn_GetPreProcessMap(FILE_DATA), vnn_GetPreProcessMapCount(FILE_DATA),
						  vnn_GetPostProcessMap(FILE_DATA), vnn_GetPostProcessMapCount(FILE_DATA), output_data_1, output_data_2,output_data_3);
		TEST_CHECK_PTR(graph, final);
    }else if(data_type == FILE_NB){
	    graph = vnn_CreateYolov3TinyUint8_NB( data_file_name, NULL,
	                      vnn_GetPreProcessMap(FILE_NB), vnn_GetPreProcessMapCount(FILE_NB),
	                      vnn_GetPostProcessMap(FILE_NB), vnn_GetPostProcessMapCount(FILE_NB), output_data_1, output_data_2, output_data_3,
	                      image_width, image_height);
	    TEST_CHECK_PTR(graph, final);
    }else{
		yolov3_tiny_print(YOLOV3_TINY_ERR, "data file should be .data or .nb\n");
    }
	tmsEnd = get_perf_count();
    msVal = (tmsEnd - tmsStart)/1000000;
    usVal = (tmsEnd - tmsStart)/1000;
    yolov3_tiny_print(YOLOV3_TINY_INFO, "Create Neural Network: %llums or %lluus\n", msVal, usVal);

final:
    return graph;
}


static void set_graph_busy(int id)
{
	pthread_mutex_lock(&vnn_wrapper[id]->mutex);
	vnn_wrapper[id]->is_busy = true;
	pthread_mutex_unlock(&vnn_wrapper[id]->mutex);
}

static void set_graph_idle(int id)
{
	pthread_mutex_lock(&vnn_wrapper[id]->mutex);
	vnn_wrapper[id]->is_busy = false;
	pthread_mutex_unlock(&vnn_wrapper[id]->mutex);
}


static int find_idle_graph()
{
	int i, id;
	bool run_end = false;
	while(!run_end){
		for(i = 0; i < YOLOV3_TINY_GRAPH_NUM; i++){
			pthread_mutex_lock(&vnn_wrapper[i]->mutex);
			if(!vnn_wrapper[i]->is_busy){
				 id = vnn_wrapper[i]->id;
				 //lyp，找到后退出循环
				 run_end = true;
				 pthread_mutex_unlock(&vnn_wrapper[i]->mutex);
				 break;
			}
			pthread_mutex_unlock(&vnn_wrapper[i]->mutex);
		}
		if(i >= YOLOV3_TINY_GRAPH_NUM){
			//yolov3_tiny_print(YOLOV3_TINY_WARN, "all graph busy, sleep 100us......\n");
			usleep(100);
		}
	}
	return id;
}


void* vnn_process_graph_thread(void *arg)
{
	vsi_status status = VSI_SUCCESS;
	int ret;
	uint64_t tmsStart, tmsEnd, msVal, usVal;
	int *index = NULL;
	int id;
#ifdef PULL_GET_TENSOR_HANDLE
	vsi_nn_tensor_t *output1_tensor;
	vsi_nn_tensor_t *output2_tensor;
#ifdef HANDLE_OUT_TENSOR
#else
	uint32_t i, sz1,sz2,stride;
	uint8_t *output1_tensor_data = NULL;
	uint8_t *output2_tensor_data = NULL;
#endif
#endif

	yolov3_tiny_print(YOLOV3_TINY_INFO, "%s create\n", __func__);

	while(1){		
		//lyp，从队列拿出要处理的graph id
		ret = FIFO_EMPTY;
		while(1){
			ret = FifoPop(process_graph_queue, (FifoObject*)&index, FIFO_EXCEPTION_ENABLE);
			if(ret == FIFO_OK){
				id = *index;
				//lyp，释放index
				if(index) free(index);
				index = NULL;
				break;
			} 
			//lyp，判断是否需要结束线程
			if(vnn_process_graph_thread_end) goto vnn_process_graph_thread_exit;
			//lyp，等待fifo数据
			usleep(100);
		}

		yolov3_tiny_print(YOLOV3_TINY_DEBUG, "%s get graph %d\n", __func__, id);

		//lyp，判断是否需要结束线程
		if(vnn_process_graph_thread_end) goto vnn_process_graph_thread_exit;
		
		//lyp，执行process graph
		tmsStart = get_perf_count();
		status = vnn_ProcessGraph( vnn_wrapper[id]->graph );
		if(status != VSI_SUCCESS){
			yolov3_tiny_print(YOLOV3_TINY_ERR, "[%s] process graph %d failed!!!\n", __func__, id);
			goto vnn_process_graph_thread_exit;
		}
		tmsEnd = get_perf_count();
		msVal = (tmsEnd - tmsStart)/1000000;
		usVal = (tmsEnd - tmsStart)/1000;
		//lyp，累积process graph时间
		vnn_process_graph_ms += msVal;
		vnn_process_graph_us += usVal;
		vnn_process_graph_ms_current[id] = ((double)tmsEnd - (double)tmsStart)/1000000;
//		yolov3_tiny_print(YOLOV3_TINY_DEBUG, "[%s] graph %d vnn_ProcessGraph: %llums or %lluus\n", __func__, id, msVal, usVal);

		//lyp，从tensor中提取推理后的数据
	#ifdef PULL_GET_TENSOR_HANDLE
	#ifdef HANDLE_OUT_TENSOR
		//lyp，获取output tensor，然后根据output tensor得到ouput data的地址
		//TODO:wc:我这里添加了一组output_data
		tmsStart = get_perf_count();
		output1_tensor = vsi_nn_GetTensor(vnn_wrapper[id]->graph, vnn_wrapper[id]->graph->output.tensors[0]);
		output2_tensor = vsi_nn_GetTensor(vnn_wrapper[id]->graph, vnn_wrapper[id]->graph->output.tensors[1]);
		vnn_wrapper[id]->output_data[0] = vnn_wrapper[id]->saved_out[0];
		vnn_wrapper[id]->output_data[1] = vnn_wrapper[id]->saved_out[1];
		vsi_nn_GetTensorHandle(output1_tensor,(void**)&(vnn_wrapper[id]->output_data[0]));
		vsi_nn_GetTensorHandle(output2_tensor,(void**)&(vnn_wrapper[id]->output_data[1]));
		tmsEnd = get_perf_count();
		msVal = (tmsEnd - tmsStart)/1000000;
		usVal = (tmsEnd - tmsStart)/1000;
		//lyp，累积process graph时间
		vnn_get_tensor_ms += msVal;
		vnn_get_tensor_us += usVal;
//		yolov3_tiny_print(YOLOV3_TINY_DEBUG, "[%s] ConvertTensorToData cost %lld ms or %lld us\n", __func__, msVal, usVal);		  
	#else
		sz1 = 1;
		output1_tensor = vsi_nn_GetTensor(vnn_wrapper[id]->graph, vnn_wrapper[id]->graph->output.tensors[0]);
		for(i = 0; i < output1_tensor->attr.dim_num; i++)
		{
			sz1 *= output1_tensor->attr.size[i];
		}
		stride = vsi_nn_TypeGetBytes(output1_tensor->attr.dtype.vx_type);
		output1_tensor_data = (uint8_t *)vsi_nn_ConvertTensorToData(vnn_wrapper[id]->graph, output1_tensor);
		vnn_wrapper[id]->output_buffer[0] = (float *)malloc(sizeof(float) * sz1);
		for(i = 0; i < sz1; i++)
		{
			status = vsi_nn_DtypeToFloat32(&output1_tensor_data[stride * i], &(vnn_wrapper[id]->output_buffer[0])[i], &output1_tensor->attr.dtype);
		}
	
		// output2 tensor
		sz2 = 1;
		output2_tensor = vsi_nn_GetTensor(vnn_wrapper[id]->graph, vnn_wrapper[id]->graph->output.tensors[1]);
		for(i = 0; i < output2_tensor->attr.dim_num; i++)
		{
		   sz2 *= output2_tensor->attr.size[i];
		}
		stride = vsi_nn_TypeGetBytes(output2_tensor->attr.dtype.vx_type);
		output2_tensor_data = (uint8_t *)vsi_nn_ConvertTensorToData(vnn_wrapper[id]->graph, output2_tensor);
		vnn_wrapper[id]->output_buffer[1] = (float *)malloc(sizeof(float) * sz2);
		for(i = 0; i < sz2; i++)
		{
		   status = vsi_nn_DtypeToFloat32(&output2_tensor_data[stride * i], &(vnn_wrapper[id]->output_buffer[1])[i], &output2_tensor->attr.dtype);
		}
	#endif
	#endif

		//lyp，tensor拷贝完后，将graph id发送给下一个步骤去处理
		index = (int*)malloc(sizeof(int));
		*index = id;
		ret = FifoPush(post_graph_queue, (FifoObject)(index), FIFO_EXCEPTION_ENABLE);
		if(ret != FIFO_OK){
			yolov3_tiny_print(YOLOV3_TINY_WARN, "[%s] FifoPush failed\n", __func__);
			free(index);
			index = NULL;
		}		
		
		//lyp，挂起线程
		usleep(50);
	}

vnn_process_graph_thread_exit:
	//lyp，取出fifo中所有数据
	while(1){
		ret = FifoPop(process_graph_queue, (FifoObject*)&index, FIFO_EXCEPTION_ENABLE);
		if(ret == FIFO_OK){
			//lyp，释放index
			if(index) free(index);
			index = NULL;
		}else break;
	}

	yolov3_tiny_print(YOLOV3_TINY_INFO, "[%s] exit\n", __func__);
	
	return NULL;
}


void* vnn_post_graph_thread(void *arg)
{
	vsi_status status = VSI_SUCCESS;
	int ret = FIFO_EMPTY;
	uint64_t tmsStart, tmsEnd, msVal, usVal;
	int id;
	detected_list list;
	detected_list *list_send = NULL;
	int *index = NULL;

	yolov3_tiny_print(YOLOV3_TINY_INFO, "%s create\n", __func__);

	while(1){
		//lyp，从队列拿出要处理的graph id
		ret = FIFO_EMPTY;
		while(1){
			ret = FifoPop(post_graph_queue, (FifoObject*)&index, FIFO_EXCEPTION_ENABLE);
			if(ret == FIFO_OK){
				id = *index;
				//lyp，释放index
				if(index) free(index);
				index = NULL;
				break;
			} 
			//lyp，判断是否需要结束线程
			if(vnn_post_graph_thread_end) goto vnn_post_graph_thread_exit;
			//lyp，等待fifo数据
			usleep(100);
		}

		yolov3_tiny_print(YOLOV3_TINY_DEBUG, "%s get graph %d\n", __func__, id);

		//lyp，判断是否需要结束线程
		if(vnn_post_graph_thread_end) goto vnn_post_graph_thread_exit;

		//lyp，执行post graph
		tmsStart = get_perf_count();
		status = vnn_PostProcessNeuralNetwork( vnn_wrapper[id], &list );
		if(status != VSI_SUCCESS){
			yolov3_tiny_print(YOLOV3_TINY_ERR, "[%s] graph %d vnn_PostProcessNeuralNetwork failed!!!\n", __func__, id);
			goto vnn_post_graph_thread_exit;
		}
		tmsEnd = get_perf_count();
		msVal = (tmsEnd - tmsStart)/1000000;
		usVal = (tmsEnd - tmsStart)/1000;
		//lyp，累积post graph时间
		vnn_post_graph_ms += msVal;
		vnn_post_graph_us += usVal;
//		yolov3_tiny_print(YOLOV3_TINY_DEBUG, "[%s] graph %d vnn_PostProcessNeuralNetwork: %llums or %lluus\n", 
//			__func__, id, msVal, usVal);

		//lyp，将结果放到fifo中(不管有没有检测到物体)
		list_send = (detected_list*)malloc(sizeof(detected_list));
		list_send->detected_class_num = list.detected_class_num;
		list_send->frame_id = list.frame_id;
		list_send->image_scaled_width = list.image_scaled_width;
		list_send->image_scaled_height = list.image_scaled_height;
//		yolov3_tiny_print(YOLOV3_TINY_WARN, "[%s] frame id: %d\n", __func__, list.frame_id);
		for(int i = 0; i < list.detected_class_num; i++){
			list_send->obj[i].left_top_x = list.obj[i].left_top_x;
			list_send->obj[i].left_top_y = list.obj[i].left_top_y;
			list_send->obj[i].right_bot_x = list.obj[i].right_bot_x;
			list_send->obj[i].right_bot_y = list.obj[i].right_bot_y;
			list_send->obj[i].prob = list.obj[i].prob;
			strcpy(list_send->obj[i].name, list.obj[i].name);
			list_send->obj[i].name_id = list.obj[i].name_id;
//			yolov3_tiny_print(YOLOV3_TINY_DEBUG, "[%s] frame %d detected %s(%d): left(%d) top(%d) right(%d) bot(%d) prob(%d)\n", 
//				__func__, list_send->frame_id, list_send->obj[i].name, list_send->obj[i].name_id, list_send->obj[i].left_top_x, 
//				list_send->obj[i].left_top_y, list_send->obj[i].right_bot_x, list_send->obj[i].right_bot_y);
		}
		ret = FifoPush(get_result_queue, (FifoObject)(list_send), FIFO_EXCEPTION_ENABLE);
		if(ret != FIFO_OK){
			yolov3_tiny_print(YOLOV3_TINY_WARN, "[%s] FifoPush failed\n", __func__);
			free(list_send);
			list_send = NULL;
		}

		//lyp，最后一个步骤要设置graph为idle
		set_graph_idle(id);
		
		//lyp，挂起线程
		usleep(100);
	}

vnn_post_graph_thread_exit:
	//lyp，取出fifo中所有数据
	while(1){
		ret = FifoPop(post_graph_queue, (FifoObject*)&index, FIFO_EXCEPTION_ENABLE);
		if(ret == FIFO_OK){
			//lyp，释放index
			if(index) free(index);
			index = NULL;
		}else break;
	}

	yolov3_tiny_print(YOLOV3_TINY_INFO, "[%s] exit\n", __func__);
	
	return NULL;
}


static data_file_type get_file_type(const char *file_name)
{
	data_file_type type;
    const char *ptr;
    char sep = '.';
    uint32_t pos,n;
    char buff[32] = {0};

    ptr = strrchr(file_name, sep);
    pos = ptr - file_name;
    n = strlen(file_name) - (pos + 1);
    strncpy(buff, file_name+(pos+1), n);

    if(strcmp(buff, "data") == 0)
    {
        type = FILE_DATA;
    }
    else if(strcmp(buff, "nb") == 0)
    {
		type = FILE_NB;
    }
    else
    {
        type = FILE_NONE;
    }

    return type;
}


int yolov3_tiny_init(yolov3_tiny_config_t* config)
{
	vsi_status status = VSI_FAILURE;
	int i = 0;
	vsi_nn_graph_t	*graph;
	uint8_t channel;

	yolov3_tiny_print(YOLOV3_TINY_INFO, "data name: %s\n", config->data_name);

	//lyp，设置log函数
	yolov3_tiny_log = config->log;

	//lyp，初始化时间
	vnn_copy_tensor_ms = 0;
	vnn_copy_tensor_us = 0;
	vnn_process_graph_ms = 0;
	vnn_process_graph_us = 0;
	vnn_post_graph_ms = 0;
	vnn_post_graph_us = 0;
	vnn_get_tensor_ms = 0;
	vnn_get_tensor_us = 0;

	//lyp，判断数据文件类型
	data_file_type data_type = get_file_type(config->data_name);
	if(data_type == FILE_NONE){
		yolov3_tiny_print(YOLOV3_TINY_ERR, "data name type err, should be .data or .nb\n");
		return -1;
	}
	yolov3_tiny_print(YOLOV3_TINY_INFO, "data name type is %s\n", data_type == FILE_DATA?"data":"nb");

	//lyp，设置前处理
	switch(config->format){
	case 0:
		channel = 3;
		break;
	case 1:
		channel = 3;
		break;
	default:
		yolov3_tiny_print(YOLOV3_TINY_ERR, "config format err: %u\n", config->format);
		return -1;
	}
	set_norm_tensor_0_size(config->width, config->height, channel);//前处理前设置tensor0的尺寸与图片大小相同
	set_roi_size(config->width, config->height, channel);
	set_norm_tensor_0_format(config->format);
	width = config->width;
	height = config->height;

	//lyp，初始化fifo
	if (FifoInit(FIFO_NUM_OF_SLOTS, &(process_graph_queue)) != FIFO_OK) {
		yolov3_tiny_print(YOLOV3_TINY_ERR, "process_graph_queue fifo init failed\n");
		goto err;
	}
	if (FifoInit(FIFO_NUM_OF_SLOTS, &(post_graph_queue)) != FIFO_OK) {
		yolov3_tiny_print(YOLOV3_TINY_ERR, "post_graph_queue fifo init failed\n");
		goto err;
	}
	if (FifoInit(FIFO_NUM_OF_SLOTS, &(get_result_queue)) != FIFO_OK) {
		yolov3_tiny_print(YOLOV3_TINY_ERR, "get_result_queue fifo init failed\n");
		goto err;
	}

#ifdef HANDLE_OUT_TENSOR
	//lyp，根据线程数创建等量的graph
	if(data_type == FILE_DATA){
		for(i = 0; i < YOLOV3_TINY_GRAPH_NUM; i++){
            vnn_wrapper[i]->output_attr[0].size[0] = 20;
            vnn_wrapper[i]->output_attr[0].size[1] = 20;
            vnn_wrapper[i]->output_attr[0].size[2] = 18;
            vnn_wrapper[i]->output_attr[0].size[3] = 1;
            vnn_wrapper[i]->output_attr[0].dim_num = 4;
            vnn_wrapper[i]->output_attr[0].dtype.scale = 0.07358036190271378;
            vnn_wrapper[i]->output_attr[0].dtype.zero_point = 164;
            vnn_wrapper[i]->output_attr[0].dtype.qnt_type = VSI_NN_QNT_TYPE_AFFINE_ASYMMETRIC;
            vnn_wrapper[i]->output_attr[0].dtype.vx_type = VSI_NN_TYPE_UINT8;

            vnn_wrapper[i]->output_data_size[0] = vsi_nn_GetTotalBytesBySize(
                    vnn_wrapper[i]->output_attr[0].size,
                    vnn_wrapper[i]->output_attr[0].dim_num,
                    vnn_wrapper[i]->output_attr[0].dtype.vx_type);
            vnn_wrapper[i]->output_data[0] = vsi_nn_MallocAlignedBuffer(
                    vnn_wrapper[i]->output_data_size[0],
                    IMAGE_ADDR_ALIGN_START_SIZE,
                    IMAGE_ADDR_ALIGN_BLOCK_SIZE);
            memset(vnn_wrapper[i]->output_data[0], 0, vnn_wrapper[i]->output_data_size[0]);
            vnn_wrapper[i]->saved_out[0] = vnn_wrapper[i]->output_data[0];

            vnn_wrapper[i]->output_attr[1].size[0] = 40;
            vnn_wrapper[i]->output_attr[1].size[1] = 40;
            vnn_wrapper[i]->output_attr[1].size[2] = 18;
            vnn_wrapper[i]->output_attr[1].size[3] = 1;
            vnn_wrapper[i]->output_attr[1].dim_num = 4;
            vnn_wrapper[i]->output_attr[1].dtype.scale = 0.07308664917945862;
            vnn_wrapper[i]->output_attr[1].dtype.zero_point = 166;
            vnn_wrapper[i]->output_attr[1].dtype.qnt_type = VSI_NN_QNT_TYPE_AFFINE_ASYMMETRIC;
            vnn_wrapper[i]->output_attr[1].dtype.vx_type = VSI_NN_TYPE_UINT8;

            vnn_wrapper[i]->output_data_size[1] = vsi_nn_GetTotalBytesBySize(
                    vnn_wrapper[i]->output_attr[1].size,
                    vnn_wrapper[i]->output_attr[1].dim_num,
                    vnn_wrapper[i]->output_attr[1].dtype.vx_type);
            vnn_wrapper[i]->output_data[1] = vsi_nn_MallocAlignedBuffer(
                    vnn_wrapper[i]->output_data_size[1],
                    IMAGE_ADDR_ALIGN_START_SIZE,
                    IMAGE_ADDR_ALIGN_BLOCK_SIZE);
            memset(vnn_wrapper[i]->output_data[1], 0, vnn_wrapper[i]->output_data_size[1]);
            vnn_wrapper[i]->saved_out[1] = vnn_wrapper[i]->output_data[1];

            vnn_wrapper[i]->output_attr[2].size[0] = 80;
            vnn_wrapper[i]->output_attr[2].size[1] = 80;
            vnn_wrapper[i]->output_attr[2].size[2] = 18;
            vnn_wrapper[i]->output_attr[2].size[3] = 1;
            vnn_wrapper[i]->output_attr[2].dim_num = 4;
            vnn_wrapper[i]->output_attr[2].dtype.scale = 0.08372680842876434;
            vnn_wrapper[i]->output_attr[2].dtype.zero_point = 164;
            vnn_wrapper[i]->output_attr[2].dtype.qnt_type = VSI_NN_QNT_TYPE_AFFINE_ASYMMETRIC;
            vnn_wrapper[i]->output_attr[2].dtype.vx_type = VSI_NN_TYPE_UINT8;

            vnn_wrapper[i]->output_data_size[2] = vsi_nn_GetTotalBytesBySize(
                    vnn_wrapper[i]->output_attr[2].size,
                    vnn_wrapper[i]->output_attr[2].dim_num,
                    vnn_wrapper[i]->output_attr[2].dtype.vx_type);
            vnn_wrapper[i]->output_data[2] = vsi_nn_MallocAlignedBuffer(
                    vnn_wrapper[i]->output_data_size[2],
                    IMAGE_ADDR_ALIGN_START_SIZE,
                    IMAGE_ADDR_ALIGN_BLOCK_SIZE);
            memset(vnn_wrapper[i]->output_data[2], 0, vnn_wrapper[i]->output_data_size[2]);
            vnn_wrapper[i]->saved_out[2] = vnn_wrapper[i]->output_data[2];
		
			//lyp，初始化mutex
			pthread_mutex_init(&vnn_wrapper[i]->mutex, NULL);
		
			//lyp，初始化使用标志
			vnn_wrapper[i]->is_busy = false;
		
			//lyp，创建神经网络
			graph = vnn_CreateNeuralNetwork( 
				config->data_name, 
				vnn_wrapper[i]->output_data[0], 
				vnn_wrapper[i]->output_data[1],
                vnn_wrapper[i]->output_data[2],
				data_type, width, height);
			vnn_wrapper[i]->graph = graph;
			yolov3_tiny_print(YOLOV3_TINY_INFO, "create graph%d(%p)\n", i, vnn_wrapper[i]->graph);
			TEST_CHECK_PTR( vnn_wrapper[i]->graph, err );
		}
	}else if(data_type == FILE_NB){
		for(i = 0; i < YOLOV3_TINY_GRAPH_NUM; i++){
			vnn_wrapper[i] = (struct Vnn_Wrapper*)calloc(1, sizeof(struct Vnn_Wrapper));

			vnn_wrapper[i]->id = i;

            vnn_wrapper[i]->output_attr[0].size[0] = 20;
            vnn_wrapper[i]->output_attr[0].size[1] = 20;
            vnn_wrapper[i]->output_attr[0].size[2] = 18;
            vnn_wrapper[i]->output_attr[0].size[3] = 1;
            vnn_wrapper[i]->output_attr[0].dim_num = 4;
            vnn_wrapper[i]->output_attr[0].dtype.scale = 0.07358036190271378;
            vnn_wrapper[i]->output_attr[0].dtype.zero_point = 164;
            vnn_wrapper[i]->output_attr[0].dtype.qnt_type = VSI_NN_QNT_TYPE_AFFINE_ASYMMETRIC;
            vnn_wrapper[i]->output_attr[0].dtype.vx_type = VSI_NN_TYPE_UINT8;

            vnn_wrapper[i]->output_data_size[0] = vsi_nn_GetTotalBytesBySize(
                    vnn_wrapper[i]->output_attr[0].size,
                    vnn_wrapper[i]->output_attr[0].dim_num,
                    vnn_wrapper[i]->output_attr[0].dtype.vx_type);
            vnn_wrapper[i]->output_data[0] = vsi_nn_MallocAlignedBuffer(
                    vnn_wrapper[i]->output_data_size[0],
                    IMAGE_ADDR_ALIGN_START_SIZE,
                    IMAGE_ADDR_ALIGN_BLOCK_SIZE);
            memset(vnn_wrapper[i]->output_data[0], 0, vnn_wrapper[i]->output_data_size[0]);
            vnn_wrapper[i]->saved_out[0] = vnn_wrapper[i]->output_data[0];

            vnn_wrapper[i]->output_attr[1].size[0] = 40;
            vnn_wrapper[i]->output_attr[1].size[1] = 40;
            vnn_wrapper[i]->output_attr[1].size[2] = 18;
            vnn_wrapper[i]->output_attr[1].size[3] = 1;
            vnn_wrapper[i]->output_attr[1].dim_num = 4;
            vnn_wrapper[i]->output_attr[1].dtype.scale = 0.07308664917945862;
            vnn_wrapper[i]->output_attr[1].dtype.zero_point = 166;
            vnn_wrapper[i]->output_attr[1].dtype.qnt_type = VSI_NN_QNT_TYPE_AFFINE_ASYMMETRIC;
            vnn_wrapper[i]->output_attr[1].dtype.vx_type = VSI_NN_TYPE_UINT8;

            vnn_wrapper[i]->output_data_size[1] = vsi_nn_GetTotalBytesBySize(
                    vnn_wrapper[i]->output_attr[1].size,
                    vnn_wrapper[i]->output_attr[1].dim_num,
                    vnn_wrapper[i]->output_attr[1].dtype.vx_type);
            vnn_wrapper[i]->output_data[1] = vsi_nn_MallocAlignedBuffer(
                    vnn_wrapper[i]->output_data_size[1],
                    IMAGE_ADDR_ALIGN_START_SIZE,
                    IMAGE_ADDR_ALIGN_BLOCK_SIZE);
            memset(vnn_wrapper[i]->output_data[1], 0, vnn_wrapper[i]->output_data_size[1]);
            vnn_wrapper[i]->saved_out[1] = vnn_wrapper[i]->output_data[1];

            vnn_wrapper[i]->output_attr[2].size[0] = 80;
            vnn_wrapper[i]->output_attr[2].size[1] = 80;
            vnn_wrapper[i]->output_attr[2].size[2] = 18;
            vnn_wrapper[i]->output_attr[2].size[3] = 1;
            vnn_wrapper[i]->output_attr[2].dim_num = 4;
            vnn_wrapper[i]->output_attr[2].dtype.scale = 0.08372680842876434;
            vnn_wrapper[i]->output_attr[2].dtype.zero_point = 164;
            vnn_wrapper[i]->output_attr[2].dtype.qnt_type = VSI_NN_QNT_TYPE_AFFINE_ASYMMETRIC;
            vnn_wrapper[i]->output_attr[2].dtype.vx_type = VSI_NN_TYPE_UINT8;

            vnn_wrapper[i]->output_data_size[2] = vsi_nn_GetTotalBytesBySize(
                    vnn_wrapper[i]->output_attr[2].size,
                    vnn_wrapper[i]->output_attr[2].dim_num,
                    vnn_wrapper[i]->output_attr[2].dtype.vx_type);
            vnn_wrapper[i]->output_data[2] = vsi_nn_MallocAlignedBuffer(
                    vnn_wrapper[i]->output_data_size[2],
                    IMAGE_ADDR_ALIGN_START_SIZE,
                    IMAGE_ADDR_ALIGN_BLOCK_SIZE);
            memset(vnn_wrapper[i]->output_data[2], 0, vnn_wrapper[i]->output_data_size[2]);
            vnn_wrapper[i]->saved_out[2] = vnn_wrapper[i]->output_data[2];

			//lyp，初始化mutex
			pthread_mutex_init(&vnn_wrapper[i]->mutex, NULL);

			//lyp，初始化使用标志
			vnn_wrapper[i]->is_busy = false;

			//lyp，创建神经网络
			graph = vnn_CreateNeuralNetwork( 
				config->data_name, 
				vnn_wrapper[i]->output_data[0], 
				vnn_wrapper[i]->output_data[1],
                vnn_wrapper[i]->output_data[2],
				data_type, width, height);
			vnn_wrapper[i]->graph = graph;
			yolov3_tiny_print(YOLOV3_TINY_INFO, "create graph%d(%p)\n", i, vnn_wrapper[i]->graph);
			TEST_CHECK_PTR( vnn_wrapper[i]->graph, err );
		}
	}

#else
	if(data_type == FILE_DATA){
		for(i = 0; i < YOLOV3_TINY_GRAPH_NUM; i++){
			vnn_wrapper[i] = (struct Vnn_Wrapper*)calloc(1, sizeof(struct Vnn_Wrapper));
			vnn_wrapper[i]->id = i;
			/* Create the neural network */
			vnn_wrapper[i]->graph = vnn_CreateNeuralNetwork(config->data_name, NULL, NULL, 
															data_type, width, height);
			TEST_CHECK_PTR( vnn_wrapper[i]->graph, err );
		}
	}else if(data_type == FILE_NB){
		for(i = 0; i < YOLOV3_TINY_GRAPH_NUM; i++){
			vnn_wrapper[i] = (struct Vnn_Wrapper*)calloc(1, sizeof(struct Vnn_Wrapper));
			vnn_wrapper[i]->id = i;
			/* Create the neural network */
			vnn_wrapper[i]->graph = vnn_CreateNeuralNetwork(config->data_name, NULL, NULL, 
															data_type, width, height);
			TEST_CHECK_PTR( vnn_wrapper[i]->graph, err );
		}
	}
#endif
	
	/* Verify graph */
	for(i = 0; i < YOLOV3_TINY_GRAPH_NUM; i++){
		status = vnn_VerifyGraph( vnn_wrapper[i]->graph );
		TEST_CHECK_STATUS( status, err);
	}

	//lyp，初始化meta数据
	load_input_meta(data_type);

	//lyp，初始化线程标志
	vnn_post_graph_thread_end = false;
	vnn_process_graph_thread_end = false;
	
	//lyp，创建graph process推理线程
	pthread_create(&vnn_progess_graph_tid, NULL, vnn_process_graph_thread, NULL);
	//lyp，创建graph post后处理线程
	pthread_create(&vnn_post_graph_tid, NULL, vnn_post_graph_thread, NULL);

	return 0;

err:
#ifdef HANDLE_OUT_TENSOR
	for(i = 0; i < YOLOV3_TINY_GRAPH_NUM; i++){
		vsi_nn_FreeAlignedBuffer(vnn_wrapper[i]->saved_out[0]);
		vsi_nn_FreeAlignedBuffer( vnn_wrapper[i]->saved_out[1]);
	}
#endif
	for(i = 0; i < YOLOV3_TINY_GRAPH_NUM; i++){
		vnn_ReleaseNeuralNetwork( vnn_wrapper[i]->graph );
		pthread_mutex_destroy(&vnn_wrapper[i]->mutex);
		free(vnn_wrapper[i]);
	}
	//lyp，释放fifo
	FifoRelease(process_graph_queue);
	FifoRelease(post_graph_queue);
	FifoRelease(get_result_queue);

	return -1;

}


int yolov3_tiny_process(uint8_t* frame, int frame_id)
{
	vsi_nn_tensor_t *tensor = NULL;
	int id;
	uint64_t tmsStart, tmsEnd, msVal, usVal;
	vsi_status status = VSI_FAILURE;
	int *index = NULL;
	int ret;

	//lyp，获取闲置graph
	id = find_idle_graph();
	//lyp，第一个步骤要设置graph为busy
	set_graph_busy(id);
	yolov3_tiny_print(YOLOV3_TINY_DEBUG, "[%s] get idle graph %d\n", __func__, id);
	//lyp，设置frame id
	vnn_wrapper[id]->frame_id = frame_id;

	//lyp，将y通道和uv通道数据分别提交给不同的tensor
	//lyp，获取tensor0
	tensor = vsi_nn_GetTensor( vnn_wrapper[id]->graph, vnn_wrapper[id]->graph->input.tensors[0] );
	//lyp，检查tensor0可容纳的数据量是否和y通道数据量匹配
	//lyp，把y通道数据发送给tensor0
	tmsStart = get_perf_count();
	status = vsi_nn_CopyDataToTensor(vnn_wrapper[id]->graph, tensor, (void*)frame);
	if(status != VSI_SUCCESS){
		yolov3_tiny_print(YOLOV3_TINY_ERR, "[%s] copy data to graph %d tensor 0 err!\n", __func__, id);
		goto copy_err;
	}
	tmsEnd = get_perf_count();
	msVal = (tmsEnd - tmsStart)/1000000;
	usVal = (tmsEnd - tmsStart)/1000;
	//lyp，累积copy tensor时间
	vnn_copy_tensor_ms += msVal;
	vnn_copy_tensor_us += usVal;
//	yolov3_tiny_print(YOLOV3_TINY_DEBUG, "[%s] Copy data to graph %d tensor 0: %llums or %lluus\n", 
//		__func__, id, msVal, usVal);
	//lyp，获取tensor1
	tmsStart = get_perf_count();
	tensor = vsi_nn_GetTensor( vnn_wrapper[id]->graph, vnn_wrapper[id]->graph->input.tensors[1] );
	//lyp，检查tensor1可容纳的数据量是否和uv通道数据量匹配
	//lyp，把uv通道数据发送给tensor1
	status = vsi_nn_CopyDataToTensor(vnn_wrapper[id]->graph, tensor, (void*)(frame+(width*height)));
	if(status != VSI_SUCCESS){
		yolov3_tiny_print(YOLOV3_TINY_ERR, "[%s] copy data to graph %d tensor 1 err!\n", __func__, id);
		goto copy_err;
	}
	tmsEnd = get_perf_count();
	msVal = (tmsEnd - tmsStart)/1000000;
	usVal = (tmsEnd - tmsStart)/1000;
	//lyp，累积copy tensor时间
	vnn_copy_tensor_ms += msVal;
	vnn_copy_tensor_us += usVal;
//	yolov3_tiny_print(YOLOV3_TINY_DEBUG, "[%s] Copy data to graph %d tensor 1: %llums or %lluus\n", 
//		__func__, id, msVal, usVal);

	//lyp，将frame放到fifo中，由线程从fifo拿去并处理
	//lyp，将buf的id通过fifo发送给vnn线程
	index = (int*)malloc(sizeof(int));
	*index = id;
	ret = FifoPush(process_graph_queue, (FifoObject)(index), FIFO_EXCEPTION_ENABLE);
	//lyp，如果满了，将buf设为空
	if(ret != FIFO_OK){
		yolov3_tiny_print(YOLOV3_TINY_WARN, "[%s] FifoPush failed\n", __func__);
		free(index);
		index = NULL;
	}

	//lyp，累计帧数
	frame_cnt++;

	return 0;

copy_err:
	//lyp，将graph设为空闲
	set_graph_idle(id);
	
	return -1;

}


int yolov3_tiny_get_result_async(user_detected_list* list)
{
	//lyp，从fifo中获取检测结果
	int ret = FIFO_EMPTY;
	detected_list *list_send = NULL;
	
	ret = FifoPop(get_result_queue, (FifoObject*)&list_send, FIFO_EXCEPTION_ENABLE);
	if(ret == FIFO_OK){
		//lyp，不管有没有检测到物体，都返回结果
		for(int i = 0; i < list_send->detected_class_num; i++){
			strcpy(list->obj[i].name, list_send->obj[i].name);
			list->obj[i].name_id = list_send->obj[i].name_id;
			list->obj[i].left_top_x = list_send->obj[i].left_top_x;
			list->obj[i].left_top_y = list_send->obj[i].left_top_y;
			list->obj[i].right_bot_x = list_send->obj[i].right_bot_x;
			list->obj[i].right_bot_y = list_send->obj[i].right_bot_y;
			list->obj[i].prob = list_send->obj[i].prob;
//			yolov3_tiny_print(YOLOV3_TINY_DEBUG, "[%s] frame %d detected %s(%d): left(%d) top(%d) right(%d) bot(%d) prob(%f)\n", 
//				__func__, list->frame_id, list->obj[i].name, list->obj[i].name_id,
//				list->obj[i].left_top_x, list->obj[i].left_top_y,
//				list->obj[i].right_bot_x, list->obj[i].right_bot_y);
		}
		list->frame_id = list_send->frame_id;
//		yolov3_tiny_print(YOLOV3_TINY_WARN, "[%s] frame id: %d\n", __func__, list_send->frame_id);
		list->detected_class_num = list_send->detected_class_num;
		list->image_scaled_width = list_send->image_scaled_width;
		list->image_scaled_height = list_send->image_scaled_height;
	} else{
//		yolov3_tiny_print(YOLOV3_TINY_ERR, "[%s] fifo pop failed: %d\n", __func__, ret);
		return ret;
	}

	//lyp，释放fifo数据
	if(list_send) free(list_send);
	list_send = NULL;

	return 0;
	
}


void yolov3_tiny_release()
{
	int i;

	yolov3_tiny_print(YOLOV3_TINY_INFO, "\n");
	yolov3_tiny_print(YOLOV3_TINY_INFO, "yolov3 tiny timeing:\n", frame_cnt);
	yolov3_tiny_print(YOLOV3_TINY_INFO, "\t frame cnt: %d\n", frame_cnt);
	yolov3_tiny_print(YOLOV3_TINY_INFO, "\t copy tensor cost %llu ms or %llu us\n", vnn_copy_tensor_ms, vnn_copy_tensor_us);
	yolov3_tiny_print(YOLOV3_TINY_INFO, "\t process graph cost %llu ms or %llu us\n", vnn_process_graph_ms, vnn_process_graph_us);
	yolov3_tiny_print(YOLOV3_TINY_INFO, "\t get tensor cost %llu ms or %llu us\n", vnn_get_tensor_ms, vnn_get_tensor_us);
	yolov3_tiny_print(YOLOV3_TINY_INFO, "\t post graph cost %llu ms or %llu us\n", vnn_post_graph_ms, vnn_post_graph_us);

	//lyp，通知结束线程
	vnn_process_graph_thread_end = true;
	vnn_post_graph_thread_end = true;

	//lyp，等待线程结束
	pthread_join(vnn_progess_graph_tid, NULL);
	pthread_join(vnn_post_graph_tid, NULL);

#ifdef HANDLE_OUT_TENSOR
	for(i = 0; i < YOLOV3_TINY_GRAPH_NUM; i++){
		vsi_nn_FreeAlignedBuffer(vnn_wrapper[i]->saved_out[0]);
		vsi_nn_FreeAlignedBuffer( vnn_wrapper[i]->saved_out[1]);
	}
#endif
	for(i = 0; i < YOLOV3_TINY_GRAPH_NUM; i++){
		pthread_mutex_destroy(&vnn_wrapper[i]->mutex);
		vnn_ReleaseNeuralNetwork( vnn_wrapper[i]->graph );
		free(vnn_wrapper[i]);
	}

	//lyp，释放fifo
	FifoRelease(process_graph_queue);
	FifoRelease(post_graph_queue);
	FifoRelease(get_result_queue);

}


double get_process_graph_time_ms(int id)
{
	if(id >= YOLOV3_TINY_GRAPH_NUM) return 0;
	
	return vnn_process_graph_ms_current[id];
}


int get_process_graph_num()
{
	return YOLOV3_TINY_GRAPH_NUM;
}


