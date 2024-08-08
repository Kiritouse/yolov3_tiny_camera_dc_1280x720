/*
 * yolov3 tiny test head file
 *
 * Maintainer: Yiping.Liang <Yiping.Liang@orbita.com>
 *
 * Copyright (C) 2021 Orbita Inc.
 *
 */


#ifndef __YOLOV3_TINY_GLOBAL_H__
#define __YOLOV3_TINY_GLOBAL_H__

#define FIFO_NUM_OF_SLOTS				128
#define USER_YOLOV3_TINY_CLASEE			1

#ifdef __cplusplus
extern "C" {
#endif

typedef struct{
	char		name[64];
	int			name_id;
	int 		left_top_x;
	int 		left_top_y;
	int 		right_bot_x;
	int 		right_bot_y;
	float		prob;
}user_detected_obj_info;

typedef struct {
	user_detected_obj_info obj[USER_YOLOV3_TINY_CLASEE];
	uint32_t image_scaled_width;	/*obj坐标以此宽度参数为参考*/
	uint32_t image_scaled_height;	/*obj坐标以此高度参数为参考*/
	int detected_class_num;
	int frame_id;
}user_detected_list;

#ifdef __cplusplus
}
#endif

#endif /* __YOLOV3_TINY_GLOBAL_H__ */

