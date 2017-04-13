##########################################################
##########################################################

TARGET := bin/signal-viewer

SRCS := src/render_hobd_obd_time.c \
    src/render_hobd_obd_uptime.c \
	src/render_hobd_obd1.c \
	src/render_hobd_obd2.c \
	src/render_hobd_obd3.c \
	src/render_hobd_heartbeat.c \
	src/render_page1.c \
	src/render_page2.c \
	src/render.c \
	src/time_domain.c \
	src/signal_table.c \
	src/display_manager.c \
	src/can.c \
	src/main.c

OBJS := $(SRCS:.c=.o)
DEPS := $(SRCS:.c=.dep)
XDEPS := $(wildcard $(DEPS))

CC = gcc

CCFLAGS = -std=gnu99

CCFLAGS += -Wall -Wextra \
          -Wformat=2 -Wno-unused-parameter -Wshadow \
          -Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
          -Wnested-externs -Wmissing-include-dirs

CCFLAGS += -DGL_GLEXT_PROTOTYPES -DGLX_GLXEXT_PROTOTYPES

LDFLAGS =

INCLUDES = -Iinclude -I../../firmware/common/include

LIBS = -lrt -lglut -lGLU -lGL -lX11 -lm -lcanlib

all: dirs $(TARGET)

ifneq ($(XDEPS),)
include $(XDEPS)
endif

dirs::
	mkdir -p bin

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

$(OBJS): %.o: %.c %.dep
	$(CC) $(CCFLAGS) $(INCLUDES) -o $@ -c $<

$(DEPS): %.dep: %.c Makefile
	$(CC) $(CCFLAGS) $(INCLUDES) -MM $< > $@

install:
	cp $(TARGET) /usr/local/bin/

clean:
	-rm -f src/*.o
	-rm -f src/*.dep
	-rm -f $(TARGET)
