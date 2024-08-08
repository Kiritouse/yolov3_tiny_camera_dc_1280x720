1，编译：
将代码复制到板子上，然后在/yolov3_tiny_camera_dc目录下直接执行make命令

2，运行：
在/yolov3_tiny_camera_dc目录下直接运行./run.sh

3，运行参数配置：
	-d			摄像头的设备号，即/dev/video*中的*，比如-d 1表示/dev/video1
	-w			摄像头采集图像的宽度，比如1280，本程序只支持720p和1080p
	-h 			摄像头采集图像的高度，比如720，本程序只支持720p和1080p
	-f			网络模型参数，支持.data和.nb文件
	-l			打印等级：0~3

4，代码中的宏配置：
	4.1，camera配置：
		4.1.1，/camera/camera_cap_usb.h 修改CAMERA_USB_OUT_BUF_MAX配置摄像头buf数，最大为4。

	4.2，显示配置：
		4.2.1，/dc8000/dc8000.h 修改DC8000_DISPLAY_FPS配置显示帧率，最高60，实际显示帧率受摄像头帧率、ai处理速度等影响。

	4.3、jpeg解码配置：
		4.3.1，/jpeg/vc8000d_jpeg.h 修改JPEGDEC_INBUF_NUM_MAX和JPEGDEC_OUTBUF_NUM_MAX可以限制解码输入buf和输出buf数量大小，但实际大小不是由这两个宏决定。
		4.3.2，jpeg解码的input buf和outbuf buf数量设置由初始化解码器时设置（调用jpegdec_open时），本程序这两个参数的设置位于主程序代码yolov3_tiny_cam_dc.cpp中的JPEGDEC_OUTBUF_NUM和JPEGDEC_INBUF_NUM。

	4.4、vip配置：
		4.4.1，为了让ai处理更流畅，可以修改/vip/yolov3_tiny.h中的YOLOV3_TINY_GRAPH_NUM配置，通过创建多个graph，并合理将每个graph用在不同的ai处理流程，可以提升ai整体的处理速度。实测YOLOV3_TINY_GRAPH_NUM只要大于2即可。

5，关于多线程优化ai处理
5.1，一般由acuity转换出来的ai处理流程为：
	a，vnn_CreateNeuralNetwork函数创建神经网络（此过程会实例化一个graph）
	b，vnn_VerifyGraph 校验神经网络
	c，vnn_PreProcessNeuralNetwork前处理，此过程的核心是通过vsi_nn_CopyDataToTensor将图像(比如rgb图像)拷贝到tensor中
	d，vnn_ProcessGraph 执行ai推理
	e，vnn_PostProcessNeuralNetwork 将推理得到的结果经过后处理获得识别目标和概率等。
	我们可以将c、d、e步骤作为线程独立出来，然后通过vnn_CreateNeuralNetwork创建3个graph，将这3个graph以流水线形式提交给c、d、e线程按顺序执行，即可提高整体的ai处理效率。	
5.2，本程序就是运用了上述的思维，把yolov3_tiny处理分成了4个部分：
	a，Copy data to tensor（位于/vip/yolov3_tiny.cpp/yolov3_tiny_process函数中），这个步骤可以由外部线程调用，比如本程序就是在/yolov3_tiny_camera_dc/yolov3_tiny_cam_dc.cpp中的yolov3_tiny_process_thread线程调用了这个函数，这里会把nv12拷贝给tensor。
	b，Process graph（位于/vip/yolov3_tiny.cpp/vnn_process_graph_thread线程中），这个步骤就是直接调用vnn_ProcessGraph进行yolov3_tiny推理。
	c，Post process（位于/vip/yolov3_tiny.cpp/vnn_post_graph_thread线程中），这个步骤就是执行后处理，得到识别物体，然后把结果保存到fifo中。
	d，Get result（位于/vip/yolov3_tiny.cpp/yolov3_tiny_get_result_async函数中），这个步骤就是从fifo中拿出识别结果，这个函数放到了display_thread线程中调用。
	实际上这四个步骤都独立为线程了，理想下同一时刻这四个线程应该在处理4个不同的graph，即以缓存方式提高了ai的整体运行速度。

		

