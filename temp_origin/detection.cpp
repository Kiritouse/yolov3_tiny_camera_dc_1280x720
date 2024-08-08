#include "detection.h"
#include "vsi_nn_pub.h"
#include "vnn_global.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include <thread>
#include <cstring>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <sys/time.h> // for gettimeofday()
#include <string.h>	  // for memset()

using namespace std;
using namespace cv;
DetectResult g_detect_objs;

/*-------------------------------------------
				  Variable definitions
-------------------------------------------*/
int output_tensor_number = 3;
// output_tensor_number=2 if yolov3-tiny,yolov4-tiny;
// output_tensor_number=3 if yolov3,yolov4,yolov5
int output_tensor_reverse = 0;
// output_tensor_reverse=0 if yolov3,yolov3-tiny,yolov4-tiny;
// output_tensor_reverse=1 if yolov4,yolov5
int classes = 1;	// class number
float thresh = 0.3; // thresh
float nms = 0.45;	// non-maximum suppression
int img_w = 640;	// image weight
int img_h = 640;	// image height
// char *image_name= "goose_1280.jpg"; //"dog.jpg";//image name
const char *image_name = "dog.jpg";
// lyp，网络可以检测到的物体
char *coco_names[] = {"explosion"};
// float biases[12] = {10,14,23,27,37,58,81,82,135,169,344,319};
float biases[18] = {12, 16, 19, 36, 40, 28, 36, 75, 76, 55, 72, 146, 142, 110, 192, 243, 459, 401}; // anchors 瞄框数值,请根据自己的进行修改
// int mask[2][3] = {{3,4,5},{0,1,2}};
int mask[3][3] = {{6, 7, 8}, {3, 4, 5}, {0, 1, 2}};

/*-------------------------------------------
				  Functions
-------------------------------------------*/

int nms_comparator(const void *pa, const void *pb)
{
	detection a = *(detection *)pa;
	detection b = *(detection *)pb;
	float diff = 0;
	if (b.sort_class >= 0)
	{
		diff = a.prob[b.sort_class] - b.prob[b.sort_class];
	}
	else
	{
		diff = a.objectness - b.objectness;
	}
	if (diff < 0)
		return 1;
	else if (diff > 0)
		return -1;
	return 0;
}

float overlap(float x1, float w1, float x2, float w2)
{
	float l1 = x1 - w1 / 2;
	float l2 = x2 - w2 / 2;
	float left = l1 > l2 ? l1 : l2;
	float r1 = x1 + w1 / 2;
	float r2 = x2 + w2 / 2;
	float right = r1 < r2 ? r1 : r2;
	return right - left;
}

float box_intersection(box a, box b)
{
	float w = overlap(a.x, a.w, b.x, b.w);
	float h = overlap(a.y, a.h, b.y, b.h);
	if (w < 0 || h < 0)
		return 0;
	float area = w * h;
	return area;
}

float box_union(box a, box b)
{
	float i = box_intersection(a, b);
	float u = a.w * a.h + b.w * b.h - i;
	return u;
}

float box_iou(box a, box b)
{
	return box_intersection(a, b) / box_union(a, b);
}

void do_nms_sort(detection *dets, int total, int classes, float thresh)
{
	int i, j, k;
	k = total - 1;
	for (i = 0; i <= k; ++i)
	{
		if (dets[i].objectness == 0)
		{
			detection swap = dets[i];
			dets[i] = dets[k];
			dets[k] = swap;
			--k;
			--i;
		}
	}
	total = k + 1;

	for (k = 0; k < classes; ++k)
	{
		for (i = 0; i < total; ++i)
		{
			dets[i].sort_class = k;
		}
		qsort(dets, total, sizeof(detection), nms_comparator);
		for (i = 0; i < total; ++i)
		{
			if (dets[i].prob[k] == 0)
				continue;
			box a = dets[i].bbox;
			for (j = i + 1; j < total; ++j)
			{
				box b = dets[j].bbox;
				if (box_iou(a, b) > thresh)
				{
					dets[j].prob[k] = 0;
				}
			}
		}
	}
}

layer make_yolo_layer(int batch, int w, int h, int n, int total, int *mask, int classes)
{
	layer l = {0};
	l.n = n;
	l.total = total;
	l.batch = batch;
	l.h = h;
	l.w = w;
	l.c = n * (classes + 4 + 1);
	l.out_w = l.w;
	l.out_h = l.h;
	l.out_c = l.c;
	l.classes = classes;
	l.inputs = l.w * l.h * l.c;

	l.biases = (float *)calloc(total * 2, sizeof(float));
	for (int i = 0; i < total * 2; ++i)
	{
		l.biases[i] = biases[i];
		// l.biases[i] = 0.5;
	}
	if (mask)
		l.mask = mask;
	else
	{
		l.mask = (int *)calloc(n, sizeof(int));
		for (int i = 0; i < n; ++i)
		{
			l.mask[i] = i;
		}
	}

	l.outputs = l.inputs;
	l.output = (float *)calloc(batch * l.outputs, sizeof(float));
	//    l.output_gpu = cuda_make_array(l.output,batch*l.outputs);
	return l;
}

void free_yolo_layer(layer l)
{
	if (NULL != l.biases)
	{
		free(l.biases);
		l.biases = NULL;
	}

	if (NULL != l.mask)
	{
		// free(l.mask);
		l.mask = NULL;
	}
	if (NULL != l.output)
	{
		free(l.output);
		l.output = NULL;
	}
}

layer *make_yolo_layer_ptr(int batch, int w, int h, int n, int total, int *mask, int classes)
{
	layer *l = new layer();
	l->n = n;
	l->total = total;
	l->batch = batch;
	l->h = h;
	l->w = w;
	l->c = n * (classes + 4 + 1);
	l->out_w = l->w;
	l->out_h = l->h;
	l->out_c = l->c;
	l->classes = classes;
	l->inputs = l->w * l->h * l->c;

	l->biases = (float *)calloc(total * 2, sizeof(float));
	for (int i = 0; i < total * 2; ++i)
	{
		l->biases[i] = biases[i];
		// l->biases[i] = 0.5;
	}
	if (mask)
		l->mask = mask;
	else
	{
		l->mask = (int *)calloc(n, sizeof(int));
		for (int i = 0; i < n; ++i)
		{
			l->mask[i] = i;
		}
	}

	l->outputs = l->inputs;
	l->output = (float *)calloc(batch * l->outputs, sizeof(float));
	//    l->output_gpu = cuda_make_array(l->output,batch*l->outputs);
	return l;
}

void free_yolo_layer_ptr(layer *l)
{
	if (NULL != l->biases)
	{
		free(l->biases);
		l->biases = NULL;
	}

	if (NULL != l->mask)
	{
		// free(l->mask);
		l->mask = NULL;
	}

	if (NULL != l->output)
	{
		free(l->output);
		l->output = NULL;
	}

	delete l;
}

static int entry_index(layer l, int batch, int location, int entry)
{
	int n = location / (l.w * l.h);
	int loc = location % (l.w * l.h);
	return batch * l.outputs + n * l.w * l.h * (4 + l.classes + 1) + entry * l.w * l.h + loc;
}

#if 0
const float __expf_rng[2] = {
	1.442695041f,
	0.693147180f
};

const float __expf_lut[8] = {
	0.9999999916728642,		//p0
	0.04165989275009526, 	//p4
	0.5000006143673624, 	//p2
	0.0014122663401803872, 	//p6
	1.000000059694879, 		//p1
	0.008336936973260111, 	//p5
	0.16666570253074878, 	//p3
	0.00019578093328483123	//p7
};

float expf_c(float x)
{
	float a, b, c, d, xx;
	int m;
	
	union {
		float   f;
		int 	i;
	} r;
		
	//Range Reduction:
	m = (int) (x * __expf_rng[0]);
	x = x - ((float) m) * __expf_rng[1];	
	
	//Taylor Polynomial (Estrins)
	a = (__expf_lut[4] * x) + (__expf_lut[0]);
	b = (__expf_lut[6] * x) + (__expf_lut[2]);
	c = (__expf_lut[5] * x) + (__expf_lut[1]);
	d = (__expf_lut[7] * x) + (__expf_lut[3]);
	xx = x * x;
	a = a + b * xx; 
	c = c + d * xx;
	xx = xx* xx;
	r.f = a + c * xx; 
	
	//multiply by 2 ^ m 
	m = m << 23;
	r.i = r.i + m;

	return r.f;
}

float expf_neon_hfp(float x)
{
//#ifdef __MATH_NEON
#if 1
	asm volatile (
	"vdup.f32 		d0, d0[0]				\n\t"	//d0 = {x, x}
	
	//Range Reduction:
	"vld1.32 		d2, [%0]				\n\t"	//d2 = {invrange, range}
	"vmul.f32 		d6, d0, d2[0]			\n\t"	//d6 = d0 * d2[0] 
	"vcvt.s32.f32 	d6, d6					\n\t"	//d6 = (int) d6
	"vcvt.f32.s32 	d1, d6					\n\t"	//d1 = (float) d6
	"vmls.f32 		d0, d1, d2[1]			\n\t"	//d0 = d0 - d1 * d2[1]
		
	//polynomial:
	"vmul.f32 		d1, d0, d0				\n\t"	//d1 = d0*d0 = {x^2, x^2}	
	"vld1.32 		{d2, d3, d4, d5}, [%1]	\n\t"	//q1 = {p0, p4, p2, p6}, q2 = {p1, p5, p3, p7} ;
	"vmla.f32 		q1, q2, d0[0]			\n\t"	//q1 = q1 + q2 * d0[0]		
	"vmla.f32 		d2, d3, d1[0]			\n\t"	//d2 = d2 + d3 * d1[0]		
	"vmul.f32 		d1, d1, d1				\n\t"	//d1 = d1 * d1 = {x^4, x^4}	
	"vmla.f32 		d2, d1, d2[1]			\n\t"	//d2 = d2 + d1 * d2[1]		

	//multiply by 2 ^ m 	
	"vshl.i32 		d6, d6, #23				\n\t"	//d6 = d6 << 23		
	"vadd.i32 		d0, d2, d6				\n\t"	//d0 = d2 + d6		

	:: "r"(__expf_rng), "r"(__expf_lut) 
    : "d0", "d1", "q1", "q2", "d6"
	);
#endif
}

float expf_neon_sfp(float x)
{
//#ifdef __MATH_NEON
#if 1
	asm volatile ("vmov.f32 s0, r0 		\n\t");
	expf_neon_hfp(x);
	asm volatile ("vmov.f32 r0, s0 		\n\t");
#else
	return expf_c(x);
#endif
}
#endif

inline float fast_exp(float x)
{
	union
	{
		uint32_t i;
		float f;
	} v;
	v.i = (1 << 23) * (1.4426950409 * x + 126.93490512f);

	return v.f;
}

#if 1
// float logistic_activate_kernel(float x){return 1.f/(1.f + expf(-x));}
float logistic_activate_kernel(float x) { return 1.f / (1.f + fast_exp(-x)); }
#else
#ifdef __cplusplus
extern "C"
{
#include <math_neon.h>
}
#endif
float logistic_activate_kernel(float x)
{
	__m128 x1, y1;
	float res;

	memcpy(&x1, &arg, sizeof(x1));
	y1 = fast_exp_sse(x1);
	memcpy(&res, &y1, sizeof(y1));

	return 1.f / (1.f + res);
	// return 1.f/(1.f + expf_neon_hfp(-x));
}
#endif

void activate_array(float *x, int n)
{
	for (int i = 0; i < n; i++)
		x[i] = logistic_activate_kernel(x[i]);
}

void forward_yolo_layer_cpu(const float *input, layer &l)
{
	// copy_gpu(l.batch*l.inputs,(float*)input,1,l.output_gpu,1);
	std::memcpy(l.output, (float *)input, l.batch * l.inputs * sizeof(float));
	int b, n;
	for (b = 0; b < l.batch; ++b)
	{
		for (n = 0; n < l.n; ++n)
		{
			int index = entry_index(l, b, n * l.w * l.h, 0);
			// activate_array_gpu(l.output_gpu + index, 2*l.w*l.h,LOGISTIC);
			activate_array(l.output + index, 2 * l.w * l.h);
			index = entry_index(l, b, n * l.w * l.h, 4);
			// activate_array_gpu(l.output_gpu + index,(1 + l.classes)*l.w*l.h,LOGISTIC);
			activate_array(l.output + index, (1 + l.classes) * l.w * l.h);
		}
	}
	//    cuda_pull_array(l.output_gpu,l.output,l.batch*l.outputs);
}

static int entry_index_ptr(layer *l, int batch, int location, int entry)
{
	int n = location / (l->w * l->h);
	int loc = location % (l->w * l->h);
	return batch * l->outputs + n * l->w * l->h * (4 + l->classes + 1) + entry * l->w * l->h + loc;
}

void forward_yolo_layer_cpu_ptr(const float *input, layer *l)
{
	// copy_gpu(l.batch*l.inputs,(float*)input,1,l.output_gpu,1);
	// lyp，拷贝数据
	std::memcpy(l->output, (float *)input, l->batch * l->inputs * sizeof(float));
	int b, n;
	for (b = 0; b < l->batch; ++b)
	{
		for (n = 0; n < l->n; ++n)
		{
			int index = entry_index_ptr(l, b, n * l->w * l->h, 0);
			// activate_array_gpu(l->output_gpu + index, 2*l->w*l->h,LOGISTIC);
			activate_array(l->output + index, 2 * l->w * l->h);
			index = entry_index_ptr(l, b, n * l->w * l->h, 4);
			// activate_array_gpu(l->output_gpu + index,(1 + l->cl->sses)*l->w*l->h,LOGISTIC);
			activate_array(l->output + index, (1 + l->classes) * l->w * l->h);
		}
	}
	//    cuda_pull_array(l.output_gpu,l.output,l.batch*l.outputs);
}

int yolo_num_detections(layer l, float thresh)
{
	int i, n;
	int count = 0;

	for (i = 0; i < l.w * l.h; ++i)
	{
		for (n = 0; n < l.n; ++n)
		{
			int obj_index = entry_index(l, 0, n * l.w * l.h + i, 4);
			if (l.output[obj_index] > thresh && l.output[obj_index] < 1.0)
			{
				++count;
			}
		}
	}
	return count;
}

int num_detections(vector<layer> layers_params, float thresh)
{
	unsigned int i;
	int s = 0;
	for (i = 0; i < layers_params.size(); ++i)
	{
		layer l = layers_params[i];
		s += yolo_num_detections(l, thresh);
	}
	return s;
}

detection *make_network_boxes(vector<layer> layers_params, float thresh, int *num)
{
	layer l = layers_params[0];
	int i;

	int nboxes = num_detections(layers_params, thresh);

	if (num)
		*num = nboxes;
	detection *dets = (detection *)calloc(nboxes, sizeof(detection));
	for (i = 0; i < nboxes; ++i)
	{
		dets[i].prob = (float *)calloc(l.classes, sizeof(float));
		// if(l.coords > 4)
		//{
		//     dets[i].mask = (float*)(l.coords-4,sizeof(float));
		// }
	}
	return dets;
}

void correct_yolo_boxes(detection *dets, int n, int w, int h, int netw, int neth, int relative)
{
	int i;
	int new_w = 0;
	int new_h = 0;
	if (((float)netw / w) < ((float)neth / h))
	{
		new_w = netw;
		new_h = (h * netw) / w;
	}
	else
	{
		new_h = neth;
		new_w = (w * neth) / h;
	}

	for (i = 0; i < n; ++i)
	{
		box b = dets[i].bbox;
		b.x = (b.x - (netw - new_w) / 2. / netw) / ((float)new_w / netw);
		b.y = (b.y - (neth - new_h) / 2. / neth) / ((float)new_h / neth);
		b.w *= (float)netw / new_w;
		b.h *= (float)neth / new_h;
		if (!relative)
		{
			b.x *= w;
			b.w *= w;
			b.y *= h;
			b.h *= h;
		}
		dets[i].bbox = b;
	}
}

box get_yolo_box(float *x, float *biases, int n, int index, int i, int j, int lw, int lh, int w, int h, int stride)
{
	// printf("biases[2*%d] = %f\n",n,biases[2*n]);
	// printf("biases[2*%d + 1] = %f\n",n,biases[2*n + 1]);
	box b;
	b.x = (i + x[index + 0 * stride]) / lw;
	b.y = (j + x[index + 1 * stride]) / lh;
	b.w = exp(x[index + 2 * stride]) * biases[2 * n] / w;
	b.h = exp(x[index + 3 * stride]) * biases[2 * n + 1] / h;
	return b;
}

int get_yolo_detections(layer l, int w, int h, int netw, int neth, float thresh, int *map, int relative, detection *dets)
{

	int i, j, n;
	float *predictions = l.output;
	int count = 0;
	for (i = 0; i < l.w * l.h; ++i)
	{
		int row = i / l.w;
		int col = i % l.w;
		for (n = 0; n < l.n; ++n)
		{

			int obj_index = entry_index(l, 0, n * l.w * l.h + i, 4);
			float objectness = predictions[obj_index]; //-nan(0x7fffff)  这里似乎越界了
			if (objectness <= thresh)
				continue;
			int box_index = entry_index(l, 0, n * l.w * l.h + i, 0);
			// printf("l.mask[%d] = %d\n",n,l.mask[n]);

			dets[count].bbox = get_yolo_box(predictions, l.biases, l.mask[n], box_index, col, row, l.w, l.h, netw, neth, l.w * l.h);

			dets[count].objectness = objectness;
			dets[count].classes = l.classes;

			for (j = 0; j < l.classes; ++j)
			{

				int class_index = entry_index(l, 0, n * l.w * l.h + i, 4 + 1 + j);

				float prob = objectness * predictions[class_index];

				dets[count].prob[j] = (prob > thresh) ? prob : 0; // TODO:这里出现了段错误
			}
			++count;
		}
	}

	correct_yolo_boxes(dets, count, w, h, netw, neth, relative);
	return count;
}

void fill_network_boxes(vector<layer> layers_params, int w, int h, int netw, int neth, float thresh, float hier, int *map, int relative, detection *dets)
{
	uint32_t j;
	for (j = 0; j < layers_params.size(); ++j)
	{
		layer l = layers_params[j];

		int count = get_yolo_detections(l, w, h, netw, neth, thresh, map, relative, dets);

		dets += count;
	}
}

detection *get_network_boxes(vector<layer> layers_params,
							 int img_w, int img_h, int netw, int neth, float thresh, float hier, int *map, int relative, int *num)
{
	// make network boxes

	detection *dets = make_network_boxes(layers_params, thresh, num);

	// fill network boxes
	fill_network_boxes(layers_params, img_w, img_h, netw, neth, thresh, hier, map, relative, dets);

	return dets;
}

typedef struct
{
	float *data;
	layer *yolo;
	int ret;
} FORWARD_LAYER_ARG_T;

void *forward_thread1(void *in_arg)
{
	FORWARD_LAYER_ARG_T *arg = (FORWARD_LAYER_ARG_T *)in_arg;
	float *data = arg->data;

	printf("do forward thread1, arg->yolo = %p, arg->yolo->batch = %d\n", arg->yolo, arg->yolo->batch);
	arg->ret = 0;
	// lyp，处理output data和layer
	forward_yolo_layer_cpu_ptr(data, arg->yolo);
	arg->ret = 1;

	return NULL;
}

void *forward_thread2(void *in_arg)
{
	FORWARD_LAYER_ARG_T *arg = (FORWARD_LAYER_ARG_T *)in_arg;
	float *data = arg->data;

	printf("do forward thread2, arg->yolo = %p, arg->yolo->batch = %d\n", arg->yolo, arg->yolo->batch);
	arg->ret = 0;
	forward_yolo_layer_cpu_ptr(data, arg->yolo);
	arg->ret = 1;

	return NULL;
}

// get detection result
detection *get_detections(vector<blob> &blobs, int img_w, int img_h, int netw, int neth, int *nboxes, int classes, float thresh, float nms)
{
	float hier_thresh = 0.5;
	vector<layer *> saved_layers_params;
	vector<layer> layers_params;
	layers_params.clear();
	//    long long total_time_ms, total_time_us;
	struct timeval start, end;

#if 0
	gettimeofday(&start, NULL);
	for(int i=0;i<blobs.size();++i){
	    layer l_params = make_yolo_layer(1, blobs[i].w, blobs[i].h, 3, sizeof(biases)/sizeof(biases[0])/2, *(mask + i), classes);
	    layers_params.push_back(l_params);

	    forward_yolo_layer_cpu(blobs[i].data, l_params);
	}
	gettimeofday(&end, NULL);
	total_time = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
	printf("1threads total time is %lld us\n", total_time);
	total_time /= 1000;
	printf("1threads total time is %lld ms\n", total_time);
#else
	FORWARD_LAYER_ARG_T *arg1 = new FORWARD_LAYER_ARG_T();
	FORWARD_LAYER_ARG_T *arg2 = new FORWARD_LAYER_ARG_T();
	pthread_t tid_1, tid_2;

	gettimeofday(&start, NULL);
	// lyp，遍历blobs元素，每个元素都指向一个output data
	for (uint32_t i = 0; i < blobs.size(); ++i)
	{
		// layer l_params = make_yolo_layer(1, blobs[i].w, blobs[i].h, 3, sizeof(biases)/sizeof(biases[0])/2, *(mask + i), classes);
		// layers_params.push_back(l_params);
		// forward_yolo_layer_gpu(blobs[i]->gpu_data(),l_params);
		// forward_yolo_layer_cpu(blobs[i].data, l_params);
		// lyp，计算layer
		layer *l_params = make_yolo_layer_ptr(1, blobs[i].w, blobs[i].h, 3, sizeof(biases) / sizeof(biases[0]) / 2, *(mask + i), classes);
		layers_params.push_back(*l_params);
		saved_layers_params.push_back(l_params);
		// lyp，每个output data放到独立的线程中处理，这里认为只有两个output data
		if (0 == i)
		{
			arg1->data = blobs[i].data;
			arg1->yolo = l_params; //&(layers_params[0]);
			pthread_create(&tid_1, NULL, forward_thread1, arg1);
			// forward_thread(&arg1);
		}
		else if (1 == i)
		{
			arg2->data = blobs[i].data;
			arg2->yolo = l_params; //&(layers_params[1]);
			pthread_create(&tid_2, NULL, forward_thread2, arg2);
			// forward_thread(&arg2);
		}
	}

	// lyp，等待两个output data处理完毕，处理完的结果保存在layers_params中
	pthread_join(tid_1, NULL);
	pthread_join(tid_2, NULL);
	gettimeofday(&end, NULL);
	//	total_time_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
	//	total_time_ms = total_time_us / 1000;
	//	printf("2threads total time is %lld ms or %lld us\n", total_time_ms, total_time_us);
	delete arg1;
	delete arg2;
#endif

	// get network boxes
	// lyp，根据layers_params以及其他信息，计算出detection
	//TODO:这里的layers_params的每个output似乎会越界
	gettimeofday(&start, NULL);
	detection *dets = get_network_boxes(layers_params, img_w, img_h, netw, neth, thresh, hier_thresh, 0, 1, nboxes);
	gettimeofday(&end, NULL);
	//	total_time_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
	//	total_time_ms = total_time_us / 1000;
	//	printf("get_network_boxes cost %lld ms or %lld us\n", total_time_ms, total_time_us);

#if 0
    //release layer memory
    for(int index =0;index < layers_params.size();++index){
        free_yolo_layer(layers_params[index]);
        //free_yolo_layer_ptr(saved_layers_params[index]);
    }
#else
	// release layer memory
	for (uint32_t index = 0; index < layers_params.size(); ++index)
	{
		// free_yolo_layer(layers_params[index]);
		free_yolo_layer_ptr(saved_layers_params[index]);
	}
#endif

	gettimeofday(&start, NULL);
	// lyp，进一步计算detection
	if (nms)
		do_nms_sort(dets, (*nboxes), classes, nms);
	gettimeofday(&end, NULL);
	//	total_time_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
	//	total_time_ms = total_time_us / 1000;
	//	printf("do_nms_sort cost %lld ms or %lld us\n", total_time_ms, total_time_us);

	// lyp，返回detection
	return dets;
}

// release detection memory
void free_detections(detection *dets, int nboxes)
{
	int i;
	for (i = 0; i < nboxes; ++i)
	{
		free(dets[i].prob);
	}
	free(dets);
}

void draw_detections()
{
	Mat img = cv::imread(image_name, 1);
	int total_num = 0;
	total_num = g_detect_objs.total_num;
	for (int i = 0; i < total_num; ++i)
	{
		string txt = g_detect_objs.name[i];
		cv::rectangle(img, g_detect_objs.pt1[i], g_detect_objs.pt2[i], cv::Scalar(10, 255, 10), 2);
		cv::Point txtOrg(g_detect_objs.pt1[i].x, g_detect_objs.pt1[i].y - 5);
		cv::putText(img, txt, txtOrg, CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 255), 1, CV_AA);
	}
	const char *save_path = "result.png";
	cv::imwrite(save_path, img);
	printf("save result to [%s].\n", save_path);
}

vsi_status show_result_3(
	struct Vnn_Wrapper *vnn_wrapper,
	vsi_nn_tensor_t *tensor,
	detected_list *list)
{
	vsi_status status = VSI_FAILURE;
	//	long long total_time_us, total_time_ms;
	//	struct timeval start, end;

#ifdef PULL_GET_TENSOR_HANDLE
	float *output1_buffer = vnn_wrapper->output_buffer[0];
	float *output2_buffer = vnn_wrapper->output_buffer[1];
	float *output3_buffer = vnn_wrapper->output_buffer[2];
#else
	vsi_nn_graph_t *graph = vnn_wrapper->graph;
	vsi_nn_tensor_t *output1_tensor;
	float *output1_buffer = NULL;
	uint8_t *output1_tensor_data = NULL;

	vsi_nn_tensor_t *output2_tensor;
	float *output2_buffer = NULL;
	uint8_t *output2_tensor_data = NULL;

	vsi_nn_tensor_t *output3_tensor;
	float *output3_buffer = NULL;
	uint8_t *output3_tensor_data = NULL;
#endif

#ifdef HANDLE_OUT_TENSOR

#else
	uint32_t i, sz1, sz2, sz3, stride;
	// lyp，获取input tensor，并得到图像宽高
	vsi_nn_tensor_t *input_tensor = NULL;
	input_tensor = vsi_nn_GetTensor(graph, graph->input.tensors[0]);
	int max_side = input_tensor->attr.size[0]; // image width
	int min_side = input_tensor->attr.size[1]; // image height
#endif

// lyp，如果没定义PULL_GET_TENSOR_HANDLE，提取tensor数据的代码放到show_result_2中执行
#ifndef PULL_GET_TENSOR_HANDLE
#ifdef HANDLE_OUT_TENSOR
	// lyp，获取output tensor，然后根据output tensor得到ouput data的地址
	//	gettimeofday(&start, NULL);
	output1_tensor = vsi_nn_GetTensor(graph, graph->output.tensors[0]);
	output2_tensor = vsi_nn_GetTensor(graph, graph->output.tensors[1]);
	output3_tensor = vsi_nn_GetTensor(graph, graph->output.tensors[2]);
	vnn_wrapper->output_data[0] = vnn_wrapper->saved_out[0];
	vnn_wrapper->output_data[1] = vnn_wrapper->saved_out[1];
	vnn_wrapper->output_data[2] = vnn_wrapper->saved_out[2];
	vsi_nn_GetTensorHandle(output1_tensor, (void **)&(vnn_wrapper->output_data[0]));
	vsi_nn_GetTensorHandle(output2_tensor, (void **)&(vnn_wrapper->output_data[1]));
	vsi_nn_GetTensorHandle(output3_tensor, (void **)&(vnn_wrapper->output_data[2]));
	//	gettimeofday(&end, NULL);
	//	total_time_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
	//	total_time_ms = total_time_us / 1000;
	// printf("ConvertTensorToData cost %lld ms or %lld us\n", total_time_ms, total_time_us);
#else
	// if (output_tensor_number==3)
	//{

	sz1 = 1;
	output1_tensor = vsi_nn_GetTensor(graph, graph->output.tensors[0]);
	for (i = 0; i < output1_tensor->attr.dim_num; i++)
	{
		sz1 *= output1_tensor->attr.size[i];
	}
	stride = vsi_nn_TypeGetBytes(output1_tensor->attr.dtype.vx_type);
	output1_tensor_data = (uint8_t *)vsi_nn_ConvertTensorToData(graph, output1_tensor);
	output1_buffer = (float *)malloc(sizeof(float) * sz1);
	for (i = 0; i < sz1; i++)
	{
		status = vsi_nn_DtypeToFloat32(&output1_tensor_data[stride * i], &output1_buffer[i], &output1_tensor->attr.dtype);
	}

	// output2 tensor
	sz2 = 1;
	output2_tensor = vsi_nn_GetTensor(graph, graph->output.tensors[1]);
	for (i = 0; i < output2_tensor->attr.dim_num; i++)
	{
		sz2 *= output2_tensor->attr.size[i];
	}
	stride = vsi_nn_TypeGetBytes(output2_tensor->attr.dtype.vx_type);
	output2_tensor_data = (uint8_t *)vsi_nn_ConvertTensorToData(graph, output2_tensor);
	output2_buffer = (float *)malloc(sizeof(float) * sz2);
	for (i = 0; i < sz2; i++)
	{
		status = vsi_nn_DtypeToFloat32(&output2_tensor_data[stride * i], &output2_buffer[i], &output2_tensor->attr.dtype);
	}

	// output3 tensor
	sz3 = 1;
	output3_tensor = vsi_nn_GetTensor(graph, graph->output.tensors[2]);
	for (i = 0; i < output3_tensor->attr.dim_num; i++)
	{
		sz3 *= output3_tensor->attr.size[i];
	}
	stride = vsi_nn_TypeGetBytes(output3_tensor->attr.dtype.vx_type);
	output3_tensor_data = (uint8_t *)vsi_nn_ConvertTensorToData(graph, output3_tensor);
	output3_buffer = (float *)malloc(sizeof(float) * sz3);
	for (i = 0; i < sz3; i++)
	{
		status = vsi_nn_DtypeToFloat32(&output3_tensor_data[stride * i], &output3_buffer[i], &output3_tensor->attr.dtype);
	}
#endif
#endif

	vector<blob> blobs;
	for (int i = 0; i < 3; i++)
	{
#ifdef HANDLE_OUT_TENSOR

		if (output_tensor_reverse == 1)
		{
			blob temp_blob = {0, 0, NULL};
			if (0 == i)
				temp_blob.data = (float *)vnn_wrapper->output_data[2];
			else if (1 == i)
				temp_blob.data = (float *)vnn_wrapper->output_data[1];
			else if (2 == i)
				temp_blob.data = (float *)vnn_wrapper->output_data[0];
			blobs.push_back(temp_blob);
		}
		else if (output_tensor_reverse == 0)
		{
			blob temp_blob = {0, 0, NULL};
			if (0 == i)
				temp_blob.data = (float *)vnn_wrapper->output_data[0];
			else if (1 == i)
				temp_blob.data = (float *)vnn_wrapper->output_data[1];
			else if (2 == i)
				temp_blob.data = (float *)vnn_wrapper->output_data[2];
			blobs.push_back(temp_blob);
		}
#else
		if (output_tensor_reverse == 1)
		{
			blob temp_blob = {0, NULL};
			if (0 == i)
				temp_blob.data = output3_buffer;
			else if (1 == i)
				temp_blob.data = output2_buffer;
			else if (2 == i)
				temp_blob.data = output1_buffer;
			blobs.push_back(temp_blob);
		}
		else if (output_tensor_reverse == 0)
		{
			blob temp_blob = {0, NULL};
			if (0 == i)
				temp_blob.data = output1_buffer;
			else if (1 == i)
				temp_blob.data = output2_buffer;
			else if (2 == i)
				temp_blob.data = output3_buffer;

			blobs.push_back(temp_blob);
		}
#endif
	}

	int netw = img_w; // img_w;
	int neth = img_h; // img_h;
	int nboxes = 0;

	detection *dets;

	blobs[0].w = netw / 32;
	blobs[1].w = netw / 16;
	blobs[2].w = netw / 8;
	blobs[0].h = neth / 32;
	blobs[1].h = neth / 16;
	blobs[2].h = neth / 8;

	dets = get_detections(blobs, img_w, img_h,
						  netw, /*network_w*/
						  neth, /*network_h*/
						  &nboxes, classes, thresh, nms);

	printf("nboxes = %d\n", nboxes);

	int detect_num = 0;
	list->detected_class_num = 0;
	list->frame_id = vnn_wrapper->frame_id;
	list->image_scaled_width = img_w;
	list->image_scaled_height = img_h;
	for (int i = 0; i < nboxes; ++i)
	{
		int cls = -1;
		float best_thresh = thresh;
		for (int j = 0; j < classes; ++j)
		{
			if (dets[i].prob[j] > best_thresh)
			{
				cls = j;
				best_thresh = dets[i].prob[j];
			}
		}

		if (cls >= 0)
		{
			box b = dets[i].bbox;

			int left = (b.x - b.w / 2.) * img_w;
			int right = (b.x + b.w / 2.) * img_w;
			int top = (b.y - b.h / 2.) * img_h;
			int bot = (b.y + b.h / 2.) * img_h;

			if (left < 0)
				left = 0;
			if (right > img_w - 1)
				right = img_w - 1;
			if (top < 0)
				top = 0;
			if (bot > img_h - 1)
				bot = img_h - 1;

			// lyp，打印检测到的物体名称和坐标，坐标由起始坐标+宽高描述
			printf("%s x(%f) y(%f) w(%f) h(%f) prob(%f)\n",
				   coco_names[cls], b.x * img_w, b.y * img_h, b.w * img_w, b.h * img_h, best_thresh);

			// lyp，保存检测结果
			memset(list->obj[list->detected_class_num].name, 0, sizeof(list->obj[list->detected_class_num].name));
			strcpy(list->obj[list->detected_class_num].name, coco_names[cls]);
			list->obj[list->detected_class_num].name_id = cls;
			list->obj[list->detected_class_num].left_top_x = left;
			list->obj[list->detected_class_num].left_top_y = top;
			list->obj[list->detected_class_num].right_bot_x = right;
			list->obj[list->detected_class_num].right_bot_y = bot;
			list->obj[list->detected_class_num].prob = best_thresh;
			list->detected_class_num++;

			// lyp，将物体名称和坐标保存到g_detect_objs中
			sprintf(g_detect_objs.name[detect_num], "%s: %.0f%%", coco_names[cls], dets[i].prob[cls] * 100);
			g_detect_objs.pt1[detect_num] = cvPoint(left, top);
			g_detect_objs.pt2[detect_num] = cvPoint(right, bot);
			detect_num++;
		}
	}

	g_detect_objs.total_num = detect_num;

	status = VSI_SUCCESS;

#ifndef PULL_GET_TENSOR_HANDLE
	if (output1_tensor_data)
		vsi_nn_Free(output1_tensor_data);
	if (output2_tensor_data)
		vsi_nn_Free(output2_tensor_data);
	if (output3_tensor_data)
		vsi_nn_Free(output3_tensor_data);
#endif
	if (output1_buffer)
		free(output1_buffer);
	if (output2_buffer)
		free(output2_buffer);
	if (output3_buffer)
		free(output3_buffer);

	free_detections(dets, nboxes);
	return status;
}

struct person
{
	int age;
	char *name;
};

struct family
{
	struct person *father;
	struct person *mother;
	int number_of_suns;
	int salary;
};

vsi_status show_result_2(
	struct Vnn_Wrapper *vnn_wrapper,
	vsi_nn_tensor_t *tensor,
	detected_list *list)
{
	vsi_status status = VSI_FAILURE;
	//	long long total_time_us, total_time_ms;
	struct timeval start, end;

#ifdef PULL_GET_TENSOR_HANDLE
	float *output1_buffer = vnn_wrapper->output_buffer[0];
	float *output2_buffer = vnn_wrapper->output_buffer[1];
#else
	vsi_nn_graph_t *graph = vnn_wrapper->graph;
	vsi_nn_tensor_t *output1_tensor;
	float *output1_buffer = NULL;
	uint8_t *output1_tensor_data = NULL;

	vsi_nn_tensor_t *output2_tensor;
	float *output2_buffer = NULL;
	uint8_t *output2_tensor_data = NULL;
#endif

#ifdef HANDLE_OUT_TENSOR

#else
	uint32_t i, sz1, sz2, stride;
	// lyp，获取input tensor，并得到图像宽高
	vsi_nn_tensor_t *input_tensor = NULL;
	input_tensor = vsi_nn_GetTensor(graph, graph->input.tensors[0]);
	int max_side = input_tensor->attr.size[0]; // image width
	int min_side = input_tensor->attr.size[1]; // image height
#endif

// lyp，如果没定义PULL_GET_TENSOR_HANDLE，提取tensor数据的代码放到show_result_2中执行
#ifndef PULL_GET_TENSOR_HANDLE
#ifdef HANDLE_OUT_TENSOR
	// lyp，获取output tensor，然后根据output tensor得到ouput data的地址
	gettimeofday(&start, NULL);
	output1_tensor = vsi_nn_GetTensor(graph, graph->output.tensors[0]);
	output2_tensor = vsi_nn_GetTensor(graph, graph->output.tensors[1]);
	vnn_wrapper->output_data[0] = vnn_wrapper->saved_out[0];
	vnn_wrapper->output_data[1] = vnn_wrapper->saved_out[1];
	vsi_nn_GetTensorHandle(output1_tensor, (void **)&(vnn_wrapper->output_data[0]));
	vsi_nn_GetTensorHandle(output2_tensor, (void **)&(vnn_wrapper->output_data[1]));
	gettimeofday(&end, NULL);
	total_time_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
	total_time_ms = total_time_us / 1000;
	printf("ConvertTensorToData cost %lld ms or %lld us\n", total_time_ms, total_time_us);
#else
	// if (output_tensor_number==3)
	//{
	sz1 = 1;
	output1_tensor = vsi_nn_GetTensor(graph, graph->output.tensors[0]);
	for (i = 0; i < output1_tensor->attr.dim_num; i++)
	{
		sz1 *= output1_tensor->attr.size[i];
	}
	stride = vsi_nn_TypeGetBytes(output1_tensor->attr.dtype.vx_type);
	output1_tensor_data = (uint8_t *)vsi_nn_ConvertTensorToData(graph, output1_tensor);
	output1_buffer = (float *)malloc(sizeof(float) * sz1);
	for (i = 0; i < sz1; i++)
	{
		status = vsi_nn_DtypeToFloat32(&output1_tensor_data[stride * i], &output1_buffer[i], &output1_tensor->attr.dtype);
	}

	// output2 tensor
	sz2 = 1;
	output2_tensor = vsi_nn_GetTensor(graph, graph->output.tensors[1]);
	for (i = 0; i < output2_tensor->attr.dim_num; i++)
	{
		sz2 *= output2_tensor->attr.size[i];
	}
	stride = vsi_nn_TypeGetBytes(output2_tensor->attr.dtype.vx_type);
	output2_tensor_data = (uint8_t *)vsi_nn_ConvertTensorToData(graph, output2_tensor);
	output2_buffer = (float *)malloc(sizeof(float) * sz2);
	for (i = 0; i < sz2; i++)
	{
		status = vsi_nn_DtypeToFloat32(&output2_tensor_data[stride * i], &output2_buffer[i], &output2_tensor->attr.dtype);
	}
#endif
#endif

	// lyp，向量blobs用于保存所有的output data
	vector<blob> blobs;
	for (int i = 0; i < 2; i++)
	{
#ifdef HANDLE_OUT_TENSOR

#if output_tensor_reverse == 1
		blob temp_blob = {0, 0, NULL};

		if (0 == i)
			temp_blob.data = (float *)vnn_wrapper->output_data[1];
		else if (1 == i)
			temp_blob.data = (float *)vnn_wrapper->output_data[0];

		blobs.push_back(temp_blob);
#else
		blob temp_blob = {0, 0, NULL};

		// lyp，blob的data成员指向output data，然后把blob作为元素放到vector中
		// lyp，vector中将会有两个blob元素，分别指向output_data_1和output_data_2
		if (0 == i)
			temp_blob.data = (float *)vnn_wrapper->output_data[0];
		else if (1 == i)
			temp_blob.data = (float *)vnn_wrapper->output_data[1];

		blobs.push_back(temp_blob);
#endif
#else

#if output_tensor_reverse == 1
		blob temp_blob = {0, 0, NULL};

		if (0 == i)
			temp_blob.data = (float *)output2_buffer;
		else if (1 == i)
			temp_blob.data = (float *)output1_buffer;

		blobs.push_back(temp_blob);
#else
		blob temp_blob = {0, 0, NULL};

		if (0 == i)
			temp_blob.data = (float *)output1_buffer;
		else if (1 == i)
			temp_blob.data = (float *)output2_buffer;

		blobs.push_back(temp_blob);
#endif
#endif
	}

#if 0
	printf("====start ==================\n");
	for(i = 0; i<5; ++i)
	{
	    printf("%f ", blobs[0].data[i]);
	}
	printf("\n");
	printf("====end ==================\n");
#endif
	int netw = img_w;
	int neth = img_h;
	int nboxes = 0;
	//	printf("img_w = %d | img_h = %d\n", img_w, img_h);

	// lyp，识别结果数组指针，可能会识别到多个物体
	detection *dets;

	blobs[0].w = netw / 32;
	blobs[1].w = netw / 16;
	blobs[0].h = neth / 32;
	blobs[1].h = neth / 16;

	// lyp，根据output data、图片宽高、以及其他信息，得到检测结果
	gettimeofday(&start, NULL);
	dets = get_detections(blobs, img_w, img_h,
						  netw, /*network_w*/
						  neth, /*network_h*/
						  &nboxes, classes, thresh, nms);
	gettimeofday(&end, NULL);
	//    total_time_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
	//    total_time_ms = total_time_us / 1000;
	//    printf("get_detections cost %lld ms or %lld us\n", total_time_ms, total_time_us);
	//}

	// lyp，打印检测到的物体数量
	//	printf("nboxes = %d\n", nboxes);

	int detect_num = 0;
	list->detected_class_num = 0;
	list->frame_id = vnn_wrapper->frame_id;
	list->image_scaled_width = img_w;
	list->image_scaled_height = img_h;
	for (int i = 0; i < nboxes; ++i)
	{
		int cls = -1;
		float best_thresh = thresh;
		// lyp，遍历所有检测种类，从中找到可能性最大的物体编号
		for (int j = 0; j < classes; ++j)
		{
			if (dets[i].prob[j] > best_thresh)
			{
				cls = j;
				best_thresh = dets[i].prob[j];
				// printf("%d: %.0f%%\n", cls, dets[i].prob[j]*100);
			}
		}

		if (cls >= 0)
		{
			box b = dets[i].bbox;

			int left = (b.x - b.w / 2.) * img_w;
			int right = (b.x + b.w / 2.) * img_w;
			int top = (b.y - b.h / 2.) * img_h;
			int bot = (b.y + b.h / 2.) * img_h;

			if (left < 0)
				left = 0;
			if (right > img_w - 1)
				right = img_w - 1;
			if (top < 0)
				top = 0;
			if (bot > img_h - 1)
				bot = img_h - 1;

			// lyp，打印检测到的物体名称和坐标，坐标由起始坐标+宽高描述
			//			printf("%s x(%f) y(%f) w(%f) h(%f) prob(%f)\n",
			//				coco_names[cls], b.x*img_w, b.y*img_h, b.w*img_w, b.h*img_h, best_thresh);

			// lyp，保存检测结果
			memset(list->obj[list->detected_class_num].name, 0, sizeof(list->obj[list->detected_class_num].name));
			strcpy(list->obj[list->detected_class_num].name, coco_names[cls]);
			list->obj[list->detected_class_num].name_id = cls;
			list->obj[list->detected_class_num].left_top_x = left;
			list->obj[list->detected_class_num].left_top_y = top;
			list->obj[list->detected_class_num].right_bot_x = right;
			list->obj[list->detected_class_num].right_bot_y = bot;
			list->obj[list->detected_class_num].prob = best_thresh;
			list->detected_class_num++;

			// lyp，将物体名称和坐标保存到g_detect_objs中
			sprintf(g_detect_objs.name[detect_num], "%s: %.0f%%", coco_names[cls], dets[i].prob[cls] * 100);
			g_detect_objs.pt1[detect_num] = cvPoint(left, top);
			g_detect_objs.pt2[detect_num] = cvPoint(right, bot);
			detect_num++;
		}
	}

	g_detect_objs.total_num = detect_num;

	status = VSI_SUCCESS;

// final:
#ifndef PULL_GET_TENSOR_HANDLE
	if (output1_tensor_data)
		vsi_nn_Free(output1_tensor_data);
	if (output2_tensor_data)
		vsi_nn_Free(output2_tensor_data);
#endif
	if (output1_buffer)
		free(output1_buffer);
	if (output2_buffer)
		free(output2_buffer);

	free_detections(dets, nboxes);
	return status;
}

vsi_status show_result(
	struct Vnn_Wrapper *vnn_wrapper,
	vsi_nn_tensor_t *tensor,
	detected_list *list)
{
	vsi_status status = VSI_FAILURE;
	vsi_nn_graph_t *graph = vnn_wrapper->graph;

	struct timeval start, end;
	gettimeofday(&start, NULL);
	if (output_tensor_number == 3)
	{
		status = show_result_3(vnn_wrapper, tensor, list);
	}
	else if (output_tensor_number == 2)
	{
		status = show_result_2(vnn_wrapper, tensor, list);
	}
	gettimeofday(&end, NULL);
	////	long long total_time = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
	////	long long total_time_us = total_time /= 1000;
	//	printf("show_result_%d time is %lld ms or %lld us\n", output_tensor_number, total_time, total_time_us);

	//	draw_detections();
	return status;
}
