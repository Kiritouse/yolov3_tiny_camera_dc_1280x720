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
#include <stdarg.h>
#include "camera_cap_usb.h"


static camera_log_func_t camera_log;
static int camera_dev;
static camera_usb_outbuf_t *out_bufs;
static uint32_t buf_cnt;

static int camera_print(camera_usb_log_t log_level, const char *fmt, ...) 
{
	va_list args;
	int i;
	char log_buf[CAMERA_USB_LOG_BUF_LEN];
	size_t size = CAMERA_USB_LOG_BUF_LEN;

	memset(log_buf, 0, size);

	va_start(args, fmt);
	i = vsnprintf(log_buf, size, fmt, args);
	va_end(args);

	if(camera_log) camera_log(log_level, log_buf);

	return i;	
}

int camera_usb_open(char * dev_name, camera_usb_config_t *config)
{
	int i;
	int ret;

	//lyp，设置log函数
	camera_log = config->log;

	if(camera_dev) camera_print(CAMERA_USB_ERROR, "camera opened\n");

	//lyp，打开摄像头
	camera_dev = open(dev_name, O_RDWR);
	if (camera_dev < 0) {
		camera_print(CAMERA_USB_ERROR, "Error opening device %s: %d.\n", dev_name, errno);	
		return camera_dev;
	}

	//lyp，获取摄像头能力
	struct v4l2_capability cap;
	memset(&cap, 0, sizeof(cap));
	ret = ioctl(camera_dev, VIDIOC_QUERYCAP, &cap);
	if (ret < 0) {
		camera_print(CAMERA_USB_ERROR, "Error opening device %s: unable to query device.\n", dev_name);
		close(camera_dev);
		return ret;
	}
	if ((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0) {
		camera_print(CAMERA_USB_ERROR, "Error open device %s: video capture not supported.\n", dev_name);
		close(camera_dev);
		return -EINVAL;
	}
	if ((cap.capabilities & V4L2_CAP_STREAMING) == 0) {
		camera_print(CAMERA_USB_ERROR, "Error open device %s:video capture streaming not supported.\n", dev_name);
		close(camera_dev);
		return -EINVAL;
	}
	camera_print(CAMERA_USB_INFO, "Device %s opened: %s.\n", dev_name, cap.card);

	//lyp，配置摄像头
	struct v4l2_format fmt;
	memset(&fmt, 0, sizeof(fmt));
	if(config->format == 0){
		fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	}else{
		fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
	}
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = config->width;
	fmt.fmt.pix.height = config->height;
	fmt.fmt.pix.field = V4L2_FIELD_NONE;
	ret = ioctl(camera_dev, VIDIOC_S_FMT, &fmt);
	if (ret < 0) {
		camera_print(CAMERA_USB_INFO, "Unable to set format: %d.\n", errno);
		return ret;
	}
	camera_print(CAMERA_USB_INFO, "Video format set: width:%u, height:%u, buffer size:%u\n",
		fmt.fmt.pix.width, fmt.fmt.pix.height, fmt.fmt.pix.sizeimage);


	//lyp，查询可申请的摄像头缓存数
	camera_print(CAMERA_USB_INFO, "expect alloc %u bufs\n", config->buf_num);	
	if (config->buf_num < 0){
		ret = -EINVAL;
		goto err;
	}	
	struct v4l2_requestbuffers rb;
	memset(&rb, 0, sizeof(rb));
	rb.count = config->buf_num;
	rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	rb.memory = V4L2_MEMORY_MMAP;
	/*call queue_setup*/
	ret = ioctl(camera_dev, VIDIOC_REQBUFS, &rb);
	if (ret < 0) {
		camera_print(CAMERA_USB_ERROR, "Unable to alloc bufs: %d.\n", errno);
		goto err;
	}
	camera_print(CAMERA_USB_INFO, "%u buffers allocated.\n", rb.count);
	out_bufs = calloc(rb.count, sizeof(camera_usb_outbuf_t)); 
	
	//lyp，缓存映射
	struct v4l2_buffer buf0;
	buf_cnt = rb.count;
	for (i = 0; i < buf_cnt; i++) {
		memset(&buf0, 0, sizeof(buf0));
		buf0.index = i;
		buf0.memory = V4L2_MEMORY_MMAP;
		buf0.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
//		if(config->phys_addr[i]) buf0.m.offset = config->phys_addr[i];
		ret = ioctl(camera_dev, VIDIOC_QUERYBUF, &buf0);
		if (ret < 0) {
			camera_print(CAMERA_USB_ERROR, "Unable to query buffer %u (%d).\n", i, errno);
			goto err;
		}
		camera_print(CAMERA_USB_INFO, "length: %u Buffer %u\n ", buf0.length, i);
		out_bufs[i].virt_addr = mmap(0, buf0.length, PROT_READ | PROT_WRITE,
				MAP_SHARED, camera_dev, buf0.m.offset);
		if(out_bufs[i].virt_addr == MAP_FAILED){
			camera_print(CAMERA_USB_ERROR, "buf %d mmap failed\n", i);
			ret = -ENOMEM;
			goto err;
		}
		out_bufs[i].index = i;
		out_bufs[i].phys_addr = buf0.m.offset;
		out_bufs[i].len = buf0.length;
//		camera_print(CAMERA_USB_INFO, "buf %d: virt_addr(0x%08x) phy_addr(0x%08x)\n ", i, out_bufs[i].virt_addr, out_bufs[i].phys_addr);
	}

	return 0;

err:
	close(camera_dev);
	camera_dev = 0;
	if(out_bufs) free(out_bufs);
	out_bufs = NULL;
	return ret;
}


int camera_usb_start()
{
	int i;
	int  ret = 0;
	
	if(!camera_dev){
		camera_print(CAMERA_USB_ERROR, "camera not opened\n");
		return -ENODEV;
	} 

	struct v4l2_buffer buf;
	for (i = 0; i < buf_cnt; i++) {
		memset(&buf, 0, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;
		ret = ioctl(camera_dev, VIDIOC_QBUF, &buf);
		if (ret < 0) {
			camera_print(CAMERA_USB_ERROR, "queue buf %d failed: %d\n", i, ret);
			return ret;
		}
		
	}

	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ioctl(camera_dev, VIDIOC_STREAMON, &type);
	if (ret < 0){
		camera_print(CAMERA_USB_ERROR, "camera stream on failed: %d\n", ret);
	} 
	return ret;
}


int camera_usb_stop()
{
	int ret;

	if(!camera_dev){
		camera_print(CAMERA_USB_ERROR, "camera not opened\n");
		return -ENODEV;
	} 
	
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ioctl(camera_dev, VIDIOC_STREAMOFF, &type);
	if (ret < 0) {
		camera_print(CAMERA_USB_ERROR, "camera stop failed: %d\n", ret);
		return ret;
	}

	return 0;
}


int camera_usb_close()
{
	int ret;
	int i;

	if(!camera_dev){
		camera_print(CAMERA_USB_ERROR, "camera not opened\n");
		return -ENODEV;
	}

	ret = camera_usb_stop();
	if(ret){
		camera_print(CAMERA_USB_ERROR, "camera stop failed: %d\n", ret);
		return ret;
	}

	struct v4l2_buffer buf;
	for (i = 0; i < buf_cnt; i++){
		memset(&buf, 0, sizeof(buf));
		buf.index = i;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		ret = ioctl(camera_dev, VIDIOC_QBUF, &buf);
		if (ret < 0) {
			camera_print(CAMERA_USB_ERROR, "queue buf %d failed: %d\n", buf.index, ret);
			return ret;
		}
		munmap(out_bufs[i].virt_addr, out_bufs[i].len);
		out_bufs[i].virt_addr = NULL;
	}

	close(camera_dev);

	free(out_bufs);
	out_bufs = NULL;

	return 0;
}


int camera_usb_get_outbuf(camera_usb_outbuf_t *outbuf)
{
	int ret = 0;
	struct v4l2_buffer buf;

	if(!camera_dev){
		camera_print(CAMERA_USB_ERROR, "camera not opened\n");
		return -ENODEV;
	}
	
	memset(&buf, 0, sizeof(buf));
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	ret = ioctl(camera_dev, VIDIOC_DQBUF, &buf);
	if(ret < 0){
		camera_print(CAMERA_USB_ERROR, "dequeue buf failed: %d\n", ret);
		return ret;
	}
	for(int i = 0; i < buf_cnt; i++){
		if(buf.index == i){
			
			break;
		}
	}
	camera_print(CAMERA_USB_DEBUG, "dequeue buf: %d\n", buf.index);
	outbuf->index = buf.index;
	outbuf->len = buf.bytesused;
	outbuf->phys_addr = out_bufs[buf.index].phys_addr;
	outbuf->virt_addr = out_bufs[buf.index].virt_addr;

	return ret;
}


int camera_usb_prepare_outbuf(camera_usb_outbuf_t *outbuf)
{
	struct v4l2_buffer buf;
	int ret = 0;

	if(!camera_dev){
		camera_print(CAMERA_USB_ERROR, "camera not opened\n");
		return -ENODEV;
	}

	memset(&buf, 0, sizeof(buf));
	buf.index = outbuf->index;
	buf.memory = V4L2_MEMORY_MMAP;
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ioctl(camera_dev, VIDIOC_QBUF, &buf);
	if (ret < 0) {
		camera_print(CAMERA_USB_ERROR, "queue buf %d failed: %d\n", buf.index, ret);
		return ret;
	}
	camera_print(CAMERA_USB_DEBUG, "queue buf: %d\n", buf.index);
	
	return ret;
}


int camera_usb_get_fps()
{
	struct v4l2_streamparm parm;
	int ret = 0;

	if(!camera_dev){
		camera_print(CAMERA_USB_ERROR, "camera not opened\n");
		return -ENODEV;
	}

	memset(&parm, 0, sizeof(parm));
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ioctl(camera_dev, VIDIOC_G_PARM, &parm);
	if (ret < 0) {
		camera_print(CAMERA_USB_ERROR, "get parm failed: %d\n", ret);
		return ret;
	}

	camera_print(CAMERA_USB_DEBUG, "get camera fps: %u/%u\n", 
		parm.parm.output.timeperframe.numerator, parm.parm.output.timeperframe.denominator);

	return (int)(parm.parm.output.timeperframe.denominator / parm.parm.output.timeperframe.numerator);

}


