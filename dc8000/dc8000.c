/*
 * DC8000 test source file
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
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>
#include "dc8000.h"
#include "fifo.h"

#define OBT_LCD_BUFF_NUM 		100   //lyp，这个值要和内核缓存的数量MAX_FRAM_BUFF保持一致
#define DC8000_HARDWARE_FPS		60	  //lyp，dc8000底层固定帧率为60fps

static dc8000_log_func_t dc8000_log;
static int pic_id;
static FifoInst frame_queue;

typedef i32 gctINT;
extern void vsi_display_destroy(void);
extern i32 vsi_display_init(u32 width, u32 height, u32 format);
extern void vsi_display_update_buf(addr_t y_addr, addr_t uv_addr);
extern gctINT viv_obt_set_properties(i32 count);
extern void vsi_obt_update_buf(unsigned int y_addr, unsigned int uv_addr, int i, int show_times);
extern gctINT viv_obt_get_frame_showcount(int *count);
//extern void vsi_obt_update_overlay(addr_t y_addr, addr_t uv_addr);

//lyp，用于帧率控制
struct ShowTimeParam{
	//lyp，确保bf_delay和n初始化时为0
	int bf_delay; 
	int n;
	int default_fps;
	int expect_fps;
};
static struct ShowTimeParam show_time_param = {0};

enum VSI_DISPLAY_FORMAT {
	VSI_DISPLAY_X4R4G4B4 = 0,
	VSI_DISPLAY_A4R4G4B4,
	VSI_DISPLAY_X1R5G5B5,
	VSI_DISPLAY_A1R5G5B5,
	VSI_DISPLAY_R5G6B5,
	VSI_DISPLAY_X8R8G8B8,
	VSI_DISPLAY_A8R8G8B8,
	VSI_DISPLAY_YUYV,
	VSI_DISPLAY_UYVY,
	VSI_DISPLAY_INDEX8,
	VSI_DISPLAY_MONOCHROME,

	VSI_DISPLAY_YV12 = 0x0f,
	VSI_DISPLAY_A8 = 0x10,
	VSI_DISPLAY_NV12 = 0x11,
	VSI_DISPLAY_NV16 = 0x12,
	VSI_DISPLAY_MAX
};


static i32 dc8000_print(dc8000_log_t log_level, const char *fmt, ...) 
{
	va_list args;
	int i;
	char log_buf[DC8000_LOG_BUF_LEN];
	size_t size = DC8000_LOG_BUF_LEN;

	memset(log_buf, 0, size);

	va_start(args, fmt);
	i = vsnprintf(log_buf, size, fmt, args);
	va_end(args);

	if(dc8000_log) dc8000_log(log_level, log_buf);

	return i;	
}


static bool setPixelClock(u32 w, u32 h)
{
    char cmd[256] = {0};
    bool bExec = FALSE;

    if( w == 1920 && h == 1080 ){
		//lyp，0000 0000 0000 00                                                                       
		//lyp，reserved		  
		//lyp, 00 0101
		//lyp, Display pixel clock divider:5:1   
		//lyp, 001 
		//lyp, Display core clock divider:2:1    
		//lyp, 1 01
		//lyp, Display APB clock divider:6:1
		//lyp, 01 1
		//lyp, Display AHB clock divider:4:1
		//lyp, 001
		//lyp, Display AXI clock divider:2:1
		sprintf(cmd, "%s", "/sbin/devmem 0xf0c20048 32 0x00005359");   
		bExec = TRUE;
	}else if( w == 1280 && h == 720 ){
	   sprintf(cmd, "%s", "/sbin/devmem 0xf0c20048 32 0x0000b359");
	   bExec = TRUE;
	}
	else if( w == 1280 && h == 960){
	   sprintf(cmd, "%s", "/sbin/devmem 0xf0c20048 32 0x00009359");
	   bExec = TRUE;
	}
	else{
	   /* add other resolution here */
	   dc8000_print(DC8000_ERR, "unsupport resolution: %ux%u\n", w, h);
	}

    if( bExec ) {
        system(cmd);
    }

    return bExec;
}


i32 mmap_image_buffer(addr_t *buf_addr, addr_t phy_addr, i32 size)
{
	i32 fd = 0;

	fd = open("/dev/mem", O_RDWR | O_NDELAY);
	if (fd < 0) {
		printf("open /dev/mem failed.");
		return -1;
	}

	*buf_addr = (u32) mmap(0, size, PROT_READ | PROT_WRITE,
			MAP_SHARED, fd, phy_addr);

	if (MAP_FAILED == (void *) *buf_addr)
		dc8000_print(DC8000_ERR, "mmap failed, addr(0x%08x),size(%d)\n", phy_addr, size);

	return fd;
}


i32 munmap_image_buffer(addr_t *buf_addr, i32 size)
{
	if (-1 == munmap((void *) *buf_addr, size)) {
		dc8000_print(DC8000_ERR, "munmap failed %d, %s\n", errno, strerror(errno));
		return -1;
	}

	return 0;
}


i32 dc8000_open(u32 width, u32 height, dc8000_log_func_t log_func)
{
	u32 dc_format;
    i32 ret = 0;
    bool _ret;

    dc8000_log = log_func;

    _ret = setPixelClock(width, height);
    if(_ret == FALSE) return -1;

    dc_format = VSI_DISPLAY_NV12;
    pic_id = 0;

	ret = vsi_display_init(width, height, dc_format);

	//lyp, 初始化dc8000内核缓存的数量,必须在dc8000_open()之后进行设置
	viv_obt_set_properties(0);

	//lyp，初始化fifo
    if (FifoInit(OBT_LCD_BUFF_NUM, &(frame_queue)) != FIFO_OK) {
		dc8000_print(DC8000_ERR, "frame_queue fifo init failed\n");
    	return -1;
  	}

  	//lyp，初始化showtime
  	show_time_param.bf_delay = 0;
  	show_time_param.n = 0;
  	show_time_param.default_fps = DC8000_HARDWARE_FPS;
  	show_time_param.expect_fps = DC8000_DISPLAY_FPS;

	dc8000_print(DC8000_INFO, "display init: %ux%u, %ufps\n", width, height, DC8000_DISPLAY_FPS);

	return ret;
}


void dc8000_close()
{
	vsi_display_destroy();
	FifoRelease(frame_queue);
}


static int getFrameShowTimes(struct ShowTimeParam* show_time_param)
{
  int show_time = 0;

  u8 is_default_fps = (show_time_param->expect_fps == show_time_param->default_fps)? 1 : 0;

	if( 0 == is_default_fps )
	{
			while( (show_time_param->n * show_time_param->expect_fps) / show_time_param->default_fps == show_time_param->bf_delay)
			{
					++show_time_param->n;
					++show_time;
			}
	
			show_time_param->n = (show_time_param->n) % show_time_param->default_fps; 	//n 取值0~59之间
			show_time_param->bf_delay = (show_time_param->n * show_time_param->expect_fps) / show_time_param->default_fps;  // bf_delay 取整,	 例如:	 n * 25/60
	}
	else
	{
	   show_time = 1;
	}
	
  return show_time;

}


//lyp，注意，显示完一张图后，剩余图片显示数将会至少为1，不会减至0
//lyp，因为显示驱动会不断拿当前寄存器中的地址一直刷新显示
//lyp，返回剩余图片数
void dc8000_display_async(addr_t y_addr, addr_t uv_addr, int addr_id)
{
	int *pt_addr_id = NULL;
	int show_cnt;
	
	//lyp，显示次数代表了帧率，比如设为2，相当于用DC8000_HARDWARE_FPS帧显示两次，
	//lyp，最终实际帧率将会是DC8000_HARDWARE_FPS/2帧
//	vsi_obt_update_buf(y_addr, uv_addr, pic_id, DC8000_HARDWARE_FPS/DC8000_DISPLAY_FPS);
	//lyp，计算本帧需要显示的次数
	show_cnt = getFrameShowTimes(&show_time_param);
	vsi_obt_update_buf(y_addr, uv_addr, pic_id, show_cnt);
	pic_id = (pic_id + 1) % OBT_LCD_BUFF_NUM;
	dc8000_print(DC8000_DEBUG, "update display buffer: y_addr(0x%08x), uv_addr(0x%08x), id(%d), show_cnt(%d)\n",
		y_addr, uv_addr, addr_id, show_cnt);
		
	//lyp，将id放到fifo中
	pt_addr_id = NULL;
	pt_addr_id = (int*)malloc(sizeof(int));
	*pt_addr_id = addr_id;
	FifoPush(frame_queue, (FifoObject)(pt_addr_id), FIFO_EXCEPTION_ENABLE);
}


//lyp，拿到的是最老的一个id
int dc8000_get_display_done_addr_id()
{
	int ret;
	uint32_t fifo_count;
	int show_count;
	int vaild_count;
	int *pt_addr_id = NULL;
	int id;
	
	//lyp，从fifo中拿出已经显示完的帧id：
	//lyp，如果get_showcount的数量和fifo数量不相等，说明已经有帧显示完了
	//lyp，已经显示完的帧数等于 fifo数量 - get_showcount数量
	//lyp，注意，必须要 fifo数量 - get_showcount数量 >= 2 才能返回最老的id
	fifo_count = FifoCount(frame_queue);
	show_count = viv_obt_get_frame_showcount(&vaild_count);
	dc8000_print(DC8000_DEBUG, "fifo_count(%u), vaild_count(%d), show_count(%d)\n", 
		fifo_count, vaild_count, show_count);
	if(fifo_count - (uint32_t)vaild_count >= 2){
		ret = FifoPop(frame_queue, (FifoObject*)&pt_addr_id, FIFO_EXCEPTION_ENABLE);
		if(ret != FIFO_OK){
			dc8000_print(DC8000_ERR, "FifoPop frame_queue failed: %d\n", ret);
			id = -1;
		}else{
			dc8000_print(DC8000_DEBUG, "get pt_addr_id: %d\n", *pt_addr_id);
			id = *pt_addr_id;
			free(pt_addr_id);
		}
	}else{
		id = -1;
	}
	
	return id;

}


u32 dc8000_display_fps()
{
	return DC8000_HARDWARE_FPS;
}


//void dc8000_obt_update_overlay(addr_t y_addr, addr_t uv_addr)
//{
//   vsi_obt_update_overlay(y_addr, uv_addr);
//}


