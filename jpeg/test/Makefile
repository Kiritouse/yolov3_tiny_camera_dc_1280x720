CROSS_COMPILE	= arm-linux-gnueabihf-
ARCH 		= -mcpu=cortex-a9

CC 		= $(CROSS_COMPILE)gcc
AR 		= $(CROSS_COMPILE)ar rcs

#vc8000d
LDFLAGS += -L ../lib/
LDLIBS += -ldecjpeg -ldwl -lcommon

LDLIBS += -lpthread -lm

TARGET := jpegdec_test

SRC_DIRS := .	\
		 ..		

INCFLAGS = $(foreach dir, $(SRC_DIRS), -I$(dir))

CFLAGS = $(INCFLAGS)

SRCS = $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.c))

OBJS = $(patsubst %.c, %.o, $(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	@$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $@

clean:
	@rm -rf $(TARGET) $(OBJS)
