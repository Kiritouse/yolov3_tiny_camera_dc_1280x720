CXX = arm-linux-gnueabihf-g++

OPENCV_INCLUDE  = /usr/include/
OPENCV_LIB_PATH = -L /usr/lib/arm-linux-gnueabihf/
OPENCV_LIBS     = -lopencv_stitching -lopencv_video -lopencv_videostab -lopencv_photo -lopencv_flann -lopencv_ml -lopencv_features2d -lopencv_superres -lopencv_objdetect -lopencv_calib3d -lopencv_imgproc -lopencv_core -lopencv_highgui -lopencv_imgcodecs -lblas -larmadillo -lpthread -lopencv_videoio

CPP_SRC_DIRS := .	\
				..  \
				
CPPSRC = $(foreach dir, $(CPP_SRC_DIRS), $(wildcard $(dir)/*.cpp))
CPPOBJS = $(patsubst %.cpp, %.o, $(CPPSRC))

INCLUDE += -I$(OPENCV_INCLUDE)
INCLUDE += $(foreach dir, $(CPP_SRC_DIRS), -I$(dir))
CPPFLAGS  = -Wall -std=c++11 -fpermissive -fPIC -lpthread -lm #-mfpu=neon -DARM_NEON_ENABLE -fopenmp -fPIC 
CPPFLAGS  += $(INCLUDE)

LIBPATH += $(OPENCV_LIB_PATH)

LIBVAR  += $(OPENCV_LIBS)           #指明需要链接静态库.a、动态库.so名称

TARGET = opencv-test

all:$(TARGET)

$(TARGET) : $(CPPOBJS)
	@echo Linking $(notdir $@)
	@$(CXX) -rdynamic -o $@ $^ $(LIBPATH) $(LIBVAR)
	
$(CPPOBJS) : %.o: %.cpp
	$(CXX) -c $(CPPFLAGS) -o $@ $<

.PHONY:clean
clean:
	rm -rf ./$(TARGET)
	rm -rf ./*.o
	rm -rf ../*.o
