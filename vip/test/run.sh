#!/bin/bash

#export CNN_PERF=0
#export NN_EXT_SHOW_PERF=1
#export NN_LAYER_DUMP=1
#export VIV_VX_DEBUG_LEVEL=1
export VIV_VX_ENABLE_TP=1
#export VIV_VX_PROFILE=0
export VNN_LOOP_TIME=1

if [ ! -f /dev/galcore ] ;then
  rmmod galcore
fi
sleep 1
insmod  ../driver/sdk/drivers/galcore.ko  registerMemBase=0xf8800000 contiguousSize=0x200000 irqLine=21 showArgs=1 recovery=1 stuckDump=2 powerManagement=1
export VIVANTE_SDK_DIR=../driver/sdk
#export VIV_VX_ENABLE_SAVE_NETWORK_BINARY=1
export LD_LIBRARY_PATH=/usr/lib/arm-linux-gnueabihf/lapack:/usr/lib/arm-linux-gnueabihf:/lib/arm-linux-gnueabihf:/usr/lib:/lib:../driver/sdk/drivers:/usr/lib/arm-linux-gnueabihf/blas/
export VSI_USE_IMAGE_PROCESS=1
#./yolov3-tiny yolov3-tiny_uint8.export.data dog.jpg
#./yolov3-tiny yolov3_uint8.export.data  goose_1280.jpg
#./yolov3-tiny yolov3-tiny_uint8.export.data y.dat uv.dat
#./yolov3-tiny ./network_binary_pid-4962_tid--1224820896.nb y.dat uv.dat
#./yolov3-tiny -d ./416x416.nb -w 416 -h 416 -f 0 -i 416x416.nv12 -l 1
#./yolov3-tiny -d ./1280x720.nb -w 1280 -h 720 -f 0 -i 1280x720.nv12 -l 1
#./yolov3-tiny -d ./1920x1080.nb -w 1920 -h 1080 -f 0 -i 1920x1080.nv12 -l 1
#./yolov3-tiny -d ./yolov3-tiny_uint8.export.data -w 416 -h 416 -f 0 -i 416x416.nv12 -l 1
#./yolov3-tiny -d ./yolov3-tiny_uint8.export.data -w 1280 -h 720 -f 0 -i 1280x720.nv12 -l 1
#./yolov3-tiny -d ./yolov3-tiny_uint8.export.data -w 1920 -h 1080 -f 0 -i 1920x1080.nv12 -l 1

#   gdb --args ./yolov3-tiny -d ./network_binary.nb -w 1280 -h 720 -f 0 -i image_1280x720.nv12 -l 1
 ./yolov3-tiny -d ./network_binary.nb -w 1280 -h 720 -f 0 -i image_1280x720.nv12 -l 1
#./yolov3-tiny -d ./yolov3_uint8.export.data -w 1280 -h 720 -f 0 -i image_1280x720.nv12 -l 1

