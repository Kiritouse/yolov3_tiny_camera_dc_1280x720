#ifndef __YOLO_LAYER_H_
#define __YOLO_LAYER_H_

#include <string>
#include <vector>
#include "vsi_nn_pub.h"
#include "vnn_global.h"
#include <opencv2/imgproc/imgproc.hpp>
using namespace cv;

#define MAX_NUM 100
typedef struct stDetectResult
{
	char name[MAX_NUM][32];
	int total_num;
	CvPoint pt1[MAX_NUM];
	CvPoint pt2[MAX_NUM];
} DetectResult;

typedef struct
{
	float x, y, w, h;
} box;

typedef struct
{
	box bbox;
	int classes;
	float *prob;
	float *mask;
	float objectness;
	int sort_class;
} detection;

typedef struct layer
{
	int batch;
	int total;
	int n, c, h, w;
	int out_n, out_c, out_h, out_w;
	int classes;
	int inputs, outputs;
	int *mask;
	float *biases;
	float *output;
	float *output_gpu;
} layer;

typedef struct blob
{
	int w;
	int h;
	float *data;
} blob;

typedef struct
{
	float dx, dy, dw, dh;
} dbox;

//vsi_status show_result(vsi_nn_graph_t *graph, vsi_nn_tensor_t *tensor);
vsi_status show_result(
	struct Vnn_Wrapper *vnn_wrapper,
	vsi_nn_tensor_t *tensor,
	detected_list* list);
#endif
