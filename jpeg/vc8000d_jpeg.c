/*
 * VC8000D jpeg decode test source file
 *
 * Maintainer: Renwei.Liu <Renwei.Liu@orbita.com>
 *
 * Copyright (C) 2020 Orbita Inc.
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "vc8000d_jpeg.h"
#include "jpegdecapi.h"
#include "dwl.h"
#include "tb_defs.h"


static const void *jpegdec_dwl;
static pthread_mutex_t jpegdec_dwl_mutex = PTHREAD_MUTEX_INITIALIZER;
static int n_dwl_instance_count = 0;

static jpegdec_log_func_t jpegdec_log;
static pthread_mutex_t jpegdec_log_mutex = PTHREAD_MUTEX_INITIALIZER;

struct TBCfg tb_cfg;

typedef struct {
	JpegDecInst jpegdec_inst;
	struct JpegDecConfig dec_cfg;

	uint32_t input_num;
	struct DWLLinearMem jpegdec_inbuf[JPEGDEC_INBUF_NUM_MAX];
	uint32_t jpegdec_inbuf_size;
	uint32_t image_width;
	uint32_t image_height;

	uint32_t output_num;
	uint32_t output_fmt;
	struct DWLLinearMem dec_outbuf[JPEGDEC_OUTBUF_NUM_MAX];
	uint32_t jpegdec_outbuf_size[JPEGDEC_OUTBUF_NUM_MAX];
	bool dec_outbuf_lock[JPEGDEC_OUTBUF_NUM_MAX];
	pthread_mutex_t lock_outbuf;
}jpegdec_wrapper;


static int jpegdec_print(jpegdec_log_t log_level, const char *fmt, ...) 
{
	va_list args;
	int i;
	char log_buf[JPEGDEC_LOG_BUF_LEN];
	size_t size = JPEGDEC_LOG_BUF_LEN;

	memset(log_buf, 0, size);

	va_start(args, fmt);
	i = vsnprintf(log_buf, size, fmt, args);
	va_end(args);

	pthread_mutex_lock(&jpegdec_log_mutex);
	if(jpegdec_log) jpegdec_log(log_level, log_buf);
	pthread_mutex_unlock(&jpegdec_log_mutex);

	return i;	
}


static int jpegdec_alloc_input_buffer(jpegdec_wrapper* wrapper)
{
	int ret;
	int i;

	for (i = 0; i < wrapper->input_num; i++) {
	  	wrapper->jpegdec_inbuf[i].mem_type = DWL_MEM_TYPE_DMA_HOST_TO_DEVICE | DWL_MEM_TYPE_CPU;
		ret = DWLMallocLinear(jpegdec_dwl, wrapper->jpegdec_inbuf_size, &(wrapper->jpegdec_inbuf[i]));
		if (ret != DWL_OK) {
			jpegdec_print(JPEGDEC_ERR, "jpegdec_inbuf[%d] alloc mem failed\n", i, ret);
			wrapper->jpegdec_inbuf[i].virtual_address = NULL;
			return ret;
		}
		jpegdec_print(JPEGDEC_DEBUG, "jpegdec_inbuf[%d] alloc %u bytes\n", i, wrapper->jpegdec_inbuf[i].size);
	}

	return 0;
}


static int jpegdec_alloc_output_buffer(jpegdec_wrapper* wrapper, int id)
{
	int ret;

	//lyp，先释放原来的output
	if(wrapper->dec_outbuf[id].virtual_address != NULL)
		DWLFreeLinear(jpegdec_dwl, &(wrapper->dec_outbuf[id]));
	wrapper->dec_outbuf[id].mem_type = DWL_MEM_TYPE_DMA_DEVICE_TO_HOST | DWL_MEM_TYPE_DPB;
	ret = DWLMallocLinear(jpegdec_dwl, wrapper->jpegdec_outbuf_size[id], &(wrapper->dec_outbuf[id]));
	if (ret != DWL_OK) {
		jpegdec_print(JPEGDEC_ERR, "dec_outbuf_y[%d] alloc mem failed\n", id, ret);
		wrapper->dec_outbuf[id].virtual_address = NULL;
		return ret;
	}
	jpegdec_print(JPEGDEC_DEBUG, "dec_outbuf_y[%d] alloc 0x%08x %u bytes\n", 
		id, wrapper->dec_outbuf[id].bus_address, wrapper->dec_outbuf[id].size);

	return 0;
}


static void jpegdec_free_input_buffer(jpegdec_wrapper* wrapper)
{
	int i;

	for (i = 0; i < wrapper->input_num; i++){
		if(wrapper->jpegdec_inbuf[i].virtual_address != NULL)
			DWLFreeLinear(jpegdec_dwl, &(wrapper->jpegdec_inbuf[i]));
	}
}


static void jpegdec_free_output_buffer(jpegdec_wrapper* wrapper, int id)
{
	if(wrapper->dec_outbuf[id].virtual_address != NULL){
		jpegdec_print(JPEGDEC_DEBUG, "free dec output %d 0x%08x\n", id, wrapper->dec_outbuf[id].bus_address);
		DWLFreeLinear(jpegdec_dwl, &(wrapper->dec_outbuf[id]));
	}
}


static void jpegdec_free_all_output_buffer(jpegdec_wrapper* wrapper)
{
	int i;

	for (i = 0; i < wrapper->output_num; i++){
		if(wrapper->dec_outbuf[i].virtual_address != NULL)
			DWLFreeLinear(jpegdec_dwl, &(wrapper->dec_outbuf[i]));
	}
}


const void* jpegdec_open(jpegdec_config_t *config)
{
	//lyp，初始化dwl
	struct DWLInitParam dwl_init;
	pthread_mutex_lock(&jpegdec_dwl_mutex);
	dwl_init.client_type = DWL_CLIENT_TYPE_JPEG_DEC;
	/* Initialize Wrapper */
	if(jpegdec_dwl){
		jpegdec_print(JPEGDEC_DEBUG, "jpegdec_dwl already init\n");
		n_dwl_instance_count++;
	}else{
		jpegdec_dwl = DWLInit(&dwl_init);
		if (jpegdec_dwl == NULL) {
			jpegdec_print(JPEGDEC_ERR, "DWLInit fail\n");
			pthread_mutex_unlock(&jpegdec_dwl_mutex);
			return NULL;
		}
		n_dwl_instance_count++;
	}
	//lyp，设置log函数
	if(jpegdec_log == NULL) jpegdec_log = config->log;
	pthread_mutex_unlock(&jpegdec_dwl_mutex);

	//lyp，申请jpegdec wrapper
	jpegdec_wrapper* wrapper = (jpegdec_wrapper*)malloc(sizeof(jpegdec_wrapper));
	if(wrapper == NULL){
		jpegdec_print(JPEGDEC_ERR, "malloc jpegdec wrapper failed!!!\n");
		return NULL;
	}
	memset(wrapper, 0, sizeof(jpegdec_wrapper));

	if(config->input_buf_num > JPEGDEC_INBUF_NUM_MAX || config->output_buf_num > JPEGDEC_OUTBUF_NUM_MAX){
		jpegdec_print(JPEGDEC_ERR, "input_num/output_num: %u/%u too large\n", 
			config->input_buf_num, config->output_buf_num);
		goto err;
	} 
	wrapper->input_num = config->input_buf_num;
	wrapper->output_num = config->output_buf_num;

	//lyp，保存配置的图像尺寸
	wrapper->image_width = config->width;
	wrapper->image_height = config->height;
	/* input buffer size: For most images 1 byte/pixel is enough for high quality JPEG */
	wrapper->jpegdec_inbuf_size = config->width * config->height;

	//lyp，输出格式，不再根据格式设置output大小，改为动态设置
	switch(config->output_fmt){
	case 0:/* nv12 */
//		wrapper->jpegdec_outbuf_size = config->width * config->height * 3 / 2;
		break;
	case 1:/* rgb */
//		wrapper->jpegdec_outbuf_size = config->width * config->height * 3;
		wrapper->dec_cfg.ppu_config[0].rgb = 1;
		wrapper->dec_cfg.ppu_config[0].rgb_format = DEC_OUT_FRM_RGB888;
//		wrapper->dec_cfg.ppu_config[0].rgb_planar = 1;
		break;
	default:
		jpegdec_print(JPEGDEC_ERR, "output fmt err: %u\n", config->output_fmt);
		goto err;
	}
	wrapper->output_fmt = config->output_fmt;

	//lyp，是否缩放
	if(config->scale_enabled){
		wrapper->dec_cfg.ppu_config[0].scale.enabled = 1;
		wrapper->dec_cfg.ppu_config[0].scale.set_by_user = 1;
		wrapper->dec_cfg.ppu_config[0].scale.width = config->scaled_width;
		wrapper->dec_cfg.ppu_config[0].scale.height = config->scaled_height;
	}

	//lyp，申请一个jpeg编码器
	JpegDecMCConfig mc_init_cfg;
	JpegDecRet ret;
	mc_init_cfg.mc_enable = 0;
	mc_init_cfg.stream_consumed_callback = NULL;
	/* Jpeg initialization */
	ret = JpegDecInit(&(wrapper->jpegdec_inst), jpegdec_dwl, DEC_NORMAL, &mc_init_cfg);
	if (ret != JPEGDEC_OK) {
		jpegdec_print(JPEGDEC_ERR, "JpegDecInit fail ret = %d\n", ret);
		goto err;
	}

	//lyp，申请缓存，只申请input，output动态申请
	ret = jpegdec_alloc_input_buffer(wrapper);
	if(ret) goto err;
	
	//lyp，初始化output mutex
	pthread_mutex_init(&wrapper->lock_outbuf, NULL);
	//lyp，设置所有output为空闲状态
	pthread_mutex_lock(&(wrapper->lock_outbuf));
	for(int i = 0; i < wrapper->output_num; i++){
		wrapper->dec_outbuf_lock[i] = FALSE;
	}
	pthread_mutex_unlock(&(wrapper->lock_outbuf));

	jpegdec_print(JPEGDEC_INFO, "jpeg decoder open success\n");

	return wrapper;	

err:
	if(wrapper){
		if(wrapper->jpegdec_inst) JpegDecRelease(wrapper->jpegdec_inst);
		jpegdec_free_input_buffer(wrapper);
		free(wrapper);
	} 
	return NULL;
}


//lyp，不断获取，直到获取到为止
static int get_idle_outbuf_id(jpegdec_wrapper* wrapper)
{
	int i;
	if(wrapper == NULL){
		jpegdec_print(JPEGDEC_ERR, "wrapper is null\n");
		return -1;
	}

	while(1){
		pthread_mutex_lock(&(wrapper->lock_outbuf));
		for(i = 0; i < wrapper->output_num; i++){
			if(wrapper->dec_outbuf_lock[i] == FALSE) break;		
		}
		pthread_mutex_unlock(&(wrapper->lock_outbuf));
		
		if(i < wrapper->output_num){
			break;
		} else{
			jpegdec_print(JPEGDEC_WARN, "all jpegdec outbuf busy, sleep 100us\n");
			usleep(100);
		} 
	}

	return i;

}


static void lock_dec_outbuf(const void* jpegdec_handle, int id)
{
	jpegdec_wrapper* wrapper = (jpegdec_wrapper* )jpegdec_handle;
	if(wrapper == NULL){
		jpegdec_print(JPEGDEC_ERR, "jpegdec_handle is null\n");
		return;
	}

	pthread_mutex_lock(&(wrapper->lock_outbuf));
	wrapper->dec_outbuf_lock[id] = TRUE;
	pthread_mutex_unlock(&(wrapper->lock_outbuf));
}


int queue_output_buf(const void* jpegdec_handle, jpegdec_output_t jpeg_output)
{
	int id;
	jpegdec_wrapper* wrapper = (jpegdec_wrapper* )jpegdec_handle;
	if(wrapper == NULL){
		jpegdec_print(JPEGDEC_ERR, "jpegdec_handle is null\n");
		return -1;
	}
	
	pthread_mutex_lock(&(wrapper->lock_outbuf));
	for(id = 0; id < wrapper->output_num; id++){
		if(jpeg_output.bus_address == wrapper->dec_outbuf[id].bus_address){
			jpegdec_print(JPEGDEC_DEBUG, "queue jpegdec outbuf %d 0x%08x\n", id, jpeg_output.bus_address);
			wrapper->dec_outbuf_lock[id] = FALSE;
			break;
		}
	}
	pthread_mutex_unlock(&(wrapper->lock_outbuf));
	if(id >= wrapper->output_num)
		jpegdec_print(JPEGDEC_WARN, "can not find jpegdec outbuf 0x%08x match\n", jpeg_output.bus_address);
	return 0;
}


int jpegdec_decode(const void* jpegdec_handle, jpegdec_input_t *jpeg_input, jpegdec_output_t *jpeg_output)
{
	JpegDecImageInfo image_info = { 0 };
	JpegDecInput jpeg_in = { 0 };
	JpegDecOutput jpeg_out = { 0 };
	JpegDecBufferInfo hbuf = { 0 };
	jpegdec_output_t output_queue = {0};
	JpegDecRet ret;

	jpegdec_wrapper* wrapper = (jpegdec_wrapper* )jpegdec_handle;
	if(wrapper == NULL){
		jpegdec_print(JPEGDEC_ERR, "jpegdec_handle is null\n");
		return -1;
	}

	//lyp，拷贝数据，如果输入数据大小比现有申请的空间大，需要释放现有空间，并重新申请空间
	if(jpeg_input->size > wrapper->jpegdec_inbuf_size){
		jpegdec_print(JPEGDEC_WARN, "input buf len %u > alloc buf %u, should realloc buf\n", 
			jpeg_input->size, wrapper->jpegdec_inbuf_size);
		jpegdec_free_input_buffer(wrapper);
		wrapper->jpegdec_inbuf_size = jpeg_input->size;
		ret = jpegdec_alloc_input_buffer(wrapper);
		if(ret){
			jpegdec_print(JPEGDEC_ERR, "realloc jpegdec input buf err\n");
			return -1;
		}
	}
	memcpy(wrapper->jpegdec_inbuf[0].virtual_address, jpeg_input->virtual_addr, jpeg_input->size);

	jpeg_in.dec_image_type = JPEGDEC_IMAGE;

	/* Pointer to the input JPEG */
	jpeg_in.stream_buffer.virtual_address = wrapper->jpegdec_inbuf[0].virtual_address;
	jpeg_in.stream_buffer.bus_address = wrapper->jpegdec_inbuf[0].bus_address;
	jpeg_in.stream_buffer.size = wrapper->jpegdec_inbuf[0].size;
	jpeg_in.stream_buffer.mem_type = wrapper->jpegdec_inbuf[0].mem_type;
	jpeg_in.stream_length = jpeg_input->size; /* JPEG length in bytes */
	jpeg_in.buffer_size = 0;

	/* New driver added */
	jpeg_in.stream_buffer.logical_size = wrapper->jpegdec_inbuf[0].logical_size;
	jpeg_in.stream = (uint8_t *) wrapper->jpegdec_inbuf[0].virtual_address;

	/* Get image information of the JPEG image */
	ret = JpegDecGetImageInfo(wrapper->jpegdec_inst, &jpeg_in, &image_info);
	if (ret != JPEGDEC_OK) {
		if(ret == JPEGDEC_INCREASE_INPUT_BUFFER){
			//lyp，说明本次传入的jpeg数据流不是完整的一帧图像，因此直接返回，等待传入剩余部分的图像	
			jpegdec_print(JPEGDEC_WARN, "wait for next stream\n");
			return ret;
		}else if(ret == JPEGDEC_FORMAT_NOT_SUPPORTED){
			jpegdec_print(JPEGDEC_WARN, "not support format\n");
			return ret;
		}else{
			//lyp, info符合解码要求，可以继续执行下一步解码，否则返回失败，允许420和422格式
			if(wrapper->image_width == image_info.output_width && wrapper->image_height == image_info.output_height &&
				(image_info.output_format == JPEGDEC_YCbCr420_SEMIPLANAR || image_info.output_format == JPEGDEC_YCbCr422_SEMIPLANAR)){
				jpegdec_print(JPEGDEC_WARN, "JpegDecGetImageInfo failed but image info match, go on\n");
			}else{
				jpegdec_print(JPEGDEC_WARN, "JpegDecGetImageInfo failed and image info not match:\n");
				jpegdec_print(JPEGDEC_WARN, "jpeg image info: \n");
				jpegdec_print(JPEGDEC_WARN, "\t image resolution(%ux%u) fmt(0x%08x) \n", 
					image_info.output_width, image_info.output_height, image_info.output_format);
				jpegdec_print(JPEGDEC_WARN, "\t image thumb(%ux%u) fmt(0x%08x) \n", 
					image_info.output_width_thumb, image_info.output_height_thumb, image_info.output_format_thumb);
			}
		}		
	}
	//lyp, 还需要再次判断一次image info，有可能get image info成功了，但是实际上信息是不符合要求的
	if(wrapper->image_width == image_info.output_width && wrapper->image_height == image_info.output_height &&
		(image_info.output_format == JPEGDEC_YCbCr420_SEMIPLANAR || image_info.output_format == JPEGDEC_YCbCr422_SEMIPLANAR)){
		jpegdec_print(JPEGDEC_DEBUG, "JpegDecGetImageInfo success and image info match, go on:\n");
		jpegdec_print(JPEGDEC_DEBUG, "jpeg image info: \n");
		jpegdec_print(JPEGDEC_DEBUG, "\t image resolution(%ux%u) fmt(0x%08x) \n", 
			image_info.output_width, image_info.output_height, image_info.output_format);
		jpegdec_print(JPEGDEC_DEBUG, "\t image thumb(%ux%u) fmt(0x%08x) \n", 
			image_info.output_width_thumb, image_info.output_height_thumb, image_info.output_format_thumb);
	}else{
		jpegdec_print(JPEGDEC_WARN, "JpegDecGetImageInfo success but image info not match:\n");
		jpegdec_print(JPEGDEC_WARN, "jpeg image info: \n");
		jpegdec_print(JPEGDEC_WARN, "\t image resolution(%ux%u) fmt(0x%08x) \n", 
			image_info.output_width, image_info.output_height, image_info.output_format);
		jpegdec_print(JPEGDEC_WARN, "\t image thumb(%ux%u) fmt(0x%08x) \n", 
			image_info.output_width_thumb, image_info.output_height_thumb, image_info.output_format_thumb);
		return -1;
	}

	wrapper->dec_cfg.ppu_config[0].enabled = 1;
//	wrapper->dec_cfg.ppu_config[0].shaper_enabled = 1;
	wrapper->dec_cfg.align = DEC_ALIGN_128B; /* default: 128 bytes alignment */
	wrapper->dec_cfg.dec_image_type = jpeg_in.dec_image_type;
	/* dec_cfg needs to be initialized before SetInfo */
	JpegDecSetInfo(wrapper->jpegdec_inst, &wrapper->dec_cfg);
	if (ret != JPEGDEC_OK) {
		jpegdec_print(JPEGDEC_ERR, "JpegDecSetInfo fail: %d\n", ret);
		return ret;
	}

	//lyp，获取解码需要的output信息
	ret = JpegDecGetBufferInfo(wrapper->jpegdec_inst, &hbuf);
	if(ret != JPEGDEC_WAITING_FOR_BUFFER && ret != JPEGDEC_OK) {
		jpegdec_print(JPEGDEC_ERR, "JpegDecGetBufferInfo err: %d\n", ret);
		return ret;
	}
	jpegdec_print(JPEGDEC_DEBUG, "buf_to_free %p, next_buf_size %d, buf_num %d\n", 
		(void *)hbuf.buf_to_free.virtual_address, hbuf.next_buf_size, hbuf.buf_num);
	//lyp，判断是否需要释放以前的output
	if(hbuf.buf_to_free.virtual_address != NULL){
		jpegdec_print(JPEGDEC_WARN, "should free used buf %p\n", (void *)hbuf.buf_to_free.virtual_address);
		DWLFreeLinear(jpegdec_dwl, &(hbuf.buf_to_free));
	}
	//lyp，获取空闲ouput id
	int id = get_idle_outbuf_id(wrapper);
	jpegdec_print(JPEGDEC_DEBUG, "get dec_outbuf %d\n", id);
	//lyp，取出dec_outbuf，用户归还前无法再次使用
	lock_dec_outbuf(jpegdec_handle, id);
	//lyp，先释放原来的output
	jpegdec_free_output_buffer(wrapper, id);
	//lyp，申请符合解码需求的output缓存
	wrapper->jpegdec_outbuf_size[id] = hbuf.next_buf_size;
	if(hbuf.buf_num > wrapper->output_num){
		wrapper->output_num = hbuf.buf_num;
		jpegdec_print(JPEGDEC_WARN, "should extern output num from %u to %u\n", 
			wrapper->output_num, hbuf.buf_num);
	}
	ret = jpegdec_alloc_output_buffer(wrapper, id);
	if(ret){
		jpegdec_print(JPEGDEC_WARN, "jpegdec_alloc_output_buffer failed\n");
		return -1;
	}
	//lyp，设置jpeg_in
	jpeg_in.picture_buffer_y.virtual_address = wrapper->dec_outbuf[id].virtual_address;
	jpeg_in.picture_buffer_y.bus_address = wrapper->dec_outbuf[id].bus_address;
	jpeg_in.picture_buffer_y.logical_size = wrapper->dec_outbuf[id].logical_size;
	jpeg_in.picture_buffer_y.mem_type = wrapper->dec_outbuf[id].mem_type;
	jpeg_in.picture_buffer_y.size = wrapper->dec_outbuf[id].size;
	
	//lyp，执行jpeg解码
	ret = JpegDecDecode(wrapper->jpegdec_inst, &jpeg_in, &jpeg_out);
	if(ret != JPEGDEC_FRAME_READY){
		jpegdec_print(JPEGDEC_WARN, "JpegDecDecode err: %d\n", ret);	
		//lyp，归还dec_outbuf
		output_queue.bus_address = jpeg_in.picture_buffer_y.bus_address;
		queue_output_buf(jpegdec_handle, output_queue);
		return ret;
	}
	
	/* set output buffer for user*/
	jpeg_output->virtual_addr = jpeg_out.pictures[0].output_picture_y.virtual_address;
	jpeg_output->bus_address = jpeg_out.pictures[0].output_picture_y.bus_address;
	switch(wrapper->output_fmt){
	case 0:/* nv12 */
		jpeg_output->size = jpeg_out.pictures[0].output_width * jpeg_out.pictures[0].output_height * 3 / 2;
		break;
	case 1:/* rgb */
		jpeg_output->size = jpeg_out.pictures[0].output_width * jpeg_out.pictures[0].output_height * 3;
		break;
	default:
		jpegdec_print(JPEGDEC_ERR, "output fmt err: %u\n", wrapper->output_fmt);
		return -1;
		break;
	}
	jpeg_output->width = jpeg_out.pictures[0].output_width;
	jpeg_output->height = jpeg_out.pictures[0].output_height;
	jpeg_output->stride = jpeg_out.pictures[0].pic_stride;
	jpeg_output->stride_ch = jpeg_out.pictures[0].pic_stride_ch;
	jpegdec_print(JPEGDEC_DEBUG, "ouput stride:%u, stride_ch:%u \n", jpeg_output->stride, jpeg_output->stride_ch);

	return 0;
}


void jpegdec_close(const void* jpegdec_handle)
{
	jpegdec_wrapper* wrapper = (jpegdec_wrapper* )jpegdec_handle;
	if(wrapper == NULL){
		jpegdec_print(JPEGDEC_ERR, "jpegdec_handle is null\n");
		return;
	}
	if(wrapper->jpegdec_inst) JpegDecRelease(wrapper->jpegdec_inst);
	jpegdec_free_input_buffer(wrapper);
	jpegdec_free_all_output_buffer(wrapper);
	pthread_mutex_destroy(&wrapper->lock_outbuf);
	free(wrapper);

	pthread_mutex_lock(&jpegdec_dwl_mutex);
	if(jpegdec_dwl){
		if(n_dwl_instance_count == 1){
			jpegdec_print(JPEGDEC_INFO, "dwl release\n");
			DWLRelease(jpegdec_dwl);
			n_dwl_instance_count--;
		}
	}
	pthread_mutex_unlock(&jpegdec_dwl_mutex);
	
	jpegdec_print(JPEGDEC_INFO, "decoder_close\n");

}


