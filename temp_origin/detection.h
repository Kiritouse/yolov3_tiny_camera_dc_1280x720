#ifndef __YOLO_LAYER_H_
#define __YOLO_LAYER_H_

#include <string>
#include <vector>
#include "vsi_nn_pub.h"
#include "vnn_global.h"
#include <opencv2/imgproc/imgproc.hpp>
using namespace cv;

//lyp，是否将提取tensor output数据的步骤放到其他地方执行
// #define PULL_GET_TENSOR_HANDLE

#define MAX_NUM 100

typedef struct stDetectResult {
  char name[MAX_NUM][32];
  int total_num;
  CvPoint pt1[MAX_NUM];
  CvPoint pt2[MAX_NUM];
}DetectResult;

typedef struct{
	//lyp，用百分比来表示识别到的物体的位置
    float x,y,w,h;
}box;

typedef struct{
    box bbox;
    int classes;
    
    //lyp，数组指针，保存了网络能够检测到的所有物体的对应的可能性大小
    float* prob;
    
    float* mask;
    float objectness;
    int sort_class;
}detection;

typedef struct layer{
    int batch;
    int total;
    int n,c,h,w;
    int out_n,out_c,out_h,out_w;
    int classes;
    int inputs,outputs;
    int *mask;
    float* biases;
    float* output;
    float* output_gpu;
}layer;

typedef struct blob{
	int w;
	int h;
	float *data;
}blob;

typedef struct{
    float dx, dy, dw, dh;
} dbox;


vsi_status show_result(struct Vnn_Wrapper *vnn_wrapper, vsi_nn_tensor_t *tensor, detected_list* list);

#endif
