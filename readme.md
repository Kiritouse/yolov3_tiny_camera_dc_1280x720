# 1，编译：
将代码复制到板子上，然后将*.nb,*.export.data的网络文件放到/下,同时拷贝驱动到/vip/driver(所拷贝的)后,在/yolov3_tiny_camera_dc目录下直接执行make命令

# 2，运行：
在/yolov3_tiny_camera_dc目录下直接运行./run.sh

# 3，运行参数配置：
	-d			摄像头的设备号，即/dev/video*中的*，比如-d 1表示/dev/video1
	-w			摄像头采集图像的宽度，比如1280，本程序只支持720p和1080p
	-h 			摄像头采集图像的高度，比如720，本程序只支持720p和1080p
	-f			网络模型参数，支持.data和.nb文件
	-l			打印等级：0~3

# 4，代码中的宏配置：
	4.1，camera配置：
		4.1.1，/camera/camera_cap_usb.h 修改CAMERA_USB_OUT_BUF_MAX配置摄像头buf数，最大为4。

	4.2，显示配置：
		4.2.1，/dc8000/dc8000.h 修改DC8000_DISPLAY_FPS配置显示帧率，最高60，实际显示帧率受摄像头帧率、ai处理速度等影响。

	4.3、jpeg解码配置：
		4.3.1，/jpeg/vc8000d_jpeg.h 修改JPEGDEC_INBUF_NUM_MAX和JPEGDEC_OUTBUF_NUM_MAX可以限制解码输入buf和输出buf数量大小，但实际大小不是由这两个宏决定。
		4.3.2，jpeg解码的input buf和outbuf buf数量设置由初始化解码器时设置（调用jpegdec_open时），本程序这两个参数的设置位于主程序代码yolov3_tiny_cam_dc.cpp中的JPEGDEC_OUTBUF_NUM和JPEGDEC_INBUF_NUM。

	4.4、vip配置：
		4.4.1，为了让ai处理更流畅，可以修改/vip/yolov3_tiny.h中的YOLOV3_TINY_GRAPH_NUM配置，通过创建多个graph，并合理将每个graph用在不同的ai处理流程，可以提升ai整体的处理速度。实测YOLOV3_TINY_GRAPH_NUM只要大于2即可。

# 5，关于多线程优化ai处理
## 5.1，一般由acuity转换出来的ai处理流程为：
	a，vnn_CreateNeuralNetwork函数创建神经网络（此过程会实例化一个graph）
	b，vnn_VerifyGraph 校验神经网络
	c，vnn_PreProcessNeuralNetwork前处理，此过程的核心是通过vsi_nn_CopyDataToTensor将图像(比如rgb图像)拷贝到tensor中
	d，vnn_ProcessGraph 执行ai推理
	e，vnn_PostProcessNeuralNetwork 将推理得到的结果经过后处理获得识别目标和概率等。
	我们可以将c、d、e步骤作为线程独立出来，然后通过vnn_CreateNeuralNetwork创建3个graph，将这3个graph以流水线形式提交给c、d、e线程按顺序执行，即可提高整体的ai处理效率。	
## 5.2，本程序就是运用了上述的思维，把yolov3_tiny处理分成了4个部分：
	a，Copy data to tensor（位于/vip/yolov3_tiny.cpp/yolov3_tiny_process函数中），这个步骤可以由外部线程调用，比如本程序就是在/yolov3_tiny_camera_dc/yolov3_tiny_cam_dc.cpp中的yolov3_tiny_process_thread线程调用了这个函数，这里会把nv12拷贝给tensor。
	b，Process graph（位于/vip/yolov3_tiny.cpp/vnn_process_graph_thread线程中），这个步骤就是直接调用vnn_ProcessGraph进行yolov3_tiny推理。
	c，Post process（位于/vip/yolov3_tiny.cpp/vnn_post_graph_thread线程中），这个步骤就是执行后处理，得到识别物体，然后把结果保存到fifo中。
	d，Get result（位于/vip/yolov3_tiny.cpp/yolov3_tiny_get_result_async函数中），这个步骤就是从fifo中拿出识别结果，这个函数放到了display_thread线程中调用。
	实际上这四个步骤都独立为线程了，理想下同一时刻这四个线程应该在处理4个不同的graph，即以缓存方式提高了ai的整体运行速度。

		
# 6, 将自己的模型移植进该项目指南
## 6.1 整体架构如下
1. 摄像头采集到的画面转为mjpeg
2. 编解码模块将每帧的画面转化为y通道和uv通道的binary data
3. 前处理(pre_process):将这些数据输入到yolo_pre_process中,然后会将这些画面数据进行缩放和归一化,转换成graph能够识别的数据,储存到每个graph->input_tensor[0](y通道数据),graph->input_tensor[1](uv通道数据)中
4. 推理(process):多线程异步推理空闲的graph,将推理的数据储存到每个graph->output_tensor[i]中(根据不同的模型,有不同的output_tensor数)
5. 后处理:(post_process):异步获取每个graph的id(通过vnn_wrapper),根据graph_>output_tensor[i]来进行前向推理,将获取到的坐标以及概率,类别和**检测时的frame_id**等相关信息储存到detected_list *list中
6. 绘制:如果当前画面的frame_id和list列表中的frame_id 不同,还是继续显示画面,如果相同,那么就根据list列表中的坐标等信息进行绘制.
> TODO:尝试用OpenCL来对OpenCV进行并行加速
## 6.2 宏
```c++
#define HANDLE_OUT_TENSOR
#define PULL_GET_TENSOR_HANDLE
```
在vnn_global中将这两个宏注释掉,我们通过graph来传递输入输出tensor,不用单独提出来

## step 1:模型转化
将模型转换成为基于ovxlib 的 cpp code,这里不多赘述,仅提供inputmeta.yml文件的配置,请根据自己的需要进行修改
```yml
# !!!This file disallow TABs!!!
# "category" allowed values: "image, frequency, undefined"
# "database" allowed types: "TEXT, NPY, H5FS, SQLITE, LMDB, GENERATOR, ZIP"
# "tensor_name" only support in H5FS database
# "preproc_type" allowed types:"IMAGE_RGB, IMAGE_RGB888_PLANAR, IMAGE_RGB888_PLANAR_SEP, IMAGE_I420, 
# IMAGE_NV12,IMAGE_NV21, IMAGE_YUV444, IMAGE_YUYV422, IMAGE_UYVY422, IMAGE_GRAY, IMAGE_BGRA, TENSOR"
input_meta:
  databases:
  - path: dataset.txt
    type: TEXT
    ports:
    - lid: images_328
      category: image # 转换过程中模型量化的所提供数据类型
      dtype: float32
      sparse: false
      tensor_name:
      layout: nchw
      shape:
      - 1
      - 3
      - 640
      - 640
      fitting: scale
      preprocess:
        reverse_channel: false
        mean:
        - 0
        - 0
        - 0
        scale: 0.003921568627451 # 这里固定为1/255
        preproc_node_params:
          add_preproc_node: true
          preproc_type: IMAGE_NV12 # 前处理输入的数据的类型
          preproc_image_size: # 前处理输入的图像的大小
          - 1280
          - 720
          preproc_crop:
            enable_preproc_crop: true # 是否启用裁剪
            crop_rect:
            - 0 #裁剪的起始坐标
            - 0
            - 1280  #裁剪的图像大小
            - 720
          preproc_perm:
          - 0
          - 1
          - 2
          - 3
      redirect_to_output: false
```

### step3: 修改生成模型代码的接口
根据demo中ai识别部分的接口来作为参考,修改生成的模型的cpp代码
ps:实际上就是将生成的在线处理和离线处理代码合在一起了,通过传入的网络的数据的data_type来进行选择

> 如果生成后的模型还要修改前处理的参数,请使用在线编译,离线编译在生成代码的时候已经和inputmeta绑定了