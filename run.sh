#!/bin/bash

#load dc8000 driver
cd ./dc8000/driver/ 
./driver_load.sh 
cd ../../ 

#load vc8000d driver
cd ./jpeg/driver/ 
./driver_load.sh 
./memalloc_load.sh 
cd ../../ 

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
insmod  ./vip/driver/sdk/drivers/galcore.ko  registerMemBase=0xf8800000 contiguousSize=0x200000 irqLine=21 showArgs=1 recovery=1 stuckDump=2 powerManagement=1
export VIVANTE_SDK_DIR=./vip/driver/sdk
#export VIV_VX_ENABLE_SAVE_NETWORK_BINARY=1
export LD_LIBRARY_PATH=/usr/lib/arm-linux-gnueabihf/lapack:/usr/lib/arm-linux-gnueabihf:/lib/arm-linux-gnueabihf:/usr/lib:/lib:./vip/driver/sdk/drivers:./dc8000/lib:/usr/lib/arm-linux-gnueabihf/blas/
export VSI_USE_IMAGE_PROCESS=1

./yolov3-tiny -d 0 -w 1280 -h 720 -f network_binary.nb -l 1  
#./yolov3-tiny -d 0 -w 1280 -h 720 -f yolov3_uint8.export.data -l 1
#./yolov3-tiny -d 0 -w 1920 -h 1080 -f 1920x1080.nb -l 1 
#./yolov3-tiny -d 0 -w 1280 -h 720 -f yolov3-tiny_uint8.export.data -l 1
#./yolov3-tiny -d 0 -w 1920 -h 1080 -f yolov3-tiny_uint8.export.data -l 1  
