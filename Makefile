CXX = arm-linux-gnueabihf-g++
CC = arm-linux-gnueabihf-gcc

VIVANTE_SDK_INCLUDE  = ./vip/driver/sdk/include/
VIVANTE_SDK_LIB_PATH = -L ./vip/driver/sdk/drivers/
LIBS     = -lOpenVX -lOpenVXU -lCLC -lVSC -lSPIRV_viv -lGAL -lovxlib -lNNArchPerf -lArchModelSw -ljpeg8
#NEON_LIBS = -L./math-neon -lmathneon

OPENCV_INCLUDE  = /usr/include/
OPENCV_LIB_PATH = -L /usr/lib/arm-linux-gnueabihf/
OPENCV_LIBS     = -lopencv_stitching -lopencv_video -lopencv_videostab -lopencv_photo -lopencv_flann -lopencv_ml -lopencv_features2d -lopencv_superres -lopencv_objdetect -lopencv_calib3d -lopencv_imgproc -lopencv_core -lopencv_highgui -lopencv_imgcodecs -lblas -larmadillo -lpthread -lopencv_videoio

CPP_SRC_DIRS := .	\
				./vip  \
				./opencv
				
C_SRC_DIRS := ./camera \
			  ./dc8000 \
			  ./jpeg \
			  ./utils

CPPSRC = $(foreach dir, $(CPP_SRC_DIRS), $(wildcard $(dir)/*.cpp))
CPPOBJS = $(patsubst %.cpp, %.o, $(CPPSRC))

CSRC = $(foreach dir, $(C_SRC_DIRS), $(wildcard $(dir)/*.c))
COBJS = $(patsubst %.c, %.o, $(CSRC))

INCLUDE += -I$(VIVANTE_SDK_INCLUDE) 
INCLUDE += -I$(OPENCV_INCLUDE)
INCLUDE += $(foreach dir, $(CPP_SRC_DIRS), -I$(dir))
INCLUDE += $(foreach dir, $(C_SRC_DIRS), -I$(dir))
CPPFLAGS  = -Wall -std=c++11 -fpermissive -fPIC -lpthread -lm #-mfpu=neon -DARM_NEON_ENABLE -fopenmp -fPIC 
CPPFLAGS  += $(INCLUDE)

CFLAGS = -Wall -std=gnu99 -lm -fPIC -lpthread
C_INCLUDE += $(foreach dir, $(C_SRC_DIRS), -I$(dir))
CFLAGS  += $(C_INCLUDE)

LIBPATH += $(VIVANTE_SDK_LIB_PATH) $(OPENCV_LIB_PATH)
LIBPATH += -L ./dc8000/lib/
LIBPATH += -L ./jpeg/lib/

LIBVAR  += $(OPENCV_LIBS) $(LIBS)          #指明需要链接静态库.a、动态库.so名称
LIBVAR  += -lviv_dc
LIBVAR  += -ldecjpeg -ldwl -lcommon

TARGET = yolov3-tiny

all:$(TARGET)

$(TARGET) : $(CPPOBJS) $(COBJS)
#	$(CXX) $(LIBPATH) $(LIBVAR) -shared -o $@
	@echo Linking $(notdir $@)
	@$(CXX) -rdynamic -o $@ $^ $(LIBPATH) $(LIBVAR)
	
#%.o:%.cpp 
#	@echo Compiling $<
#	@$(CXX)  -c $< -o $@ $(CFLAGS) $(NEON_LIBS)
	
$(COBJS) : %.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

$(CPPOBJS) : %.o: %.cpp
	$(CXX) -c $(CPPFLAGS) -o $@ $<

.PHONY:clean
clean:
	rm -rf $(TARGET) $(COBJS) $(CPPOBJS)

