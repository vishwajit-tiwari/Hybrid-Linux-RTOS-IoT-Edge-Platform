# Makefile for building the project named iot_uart_driver

# final module
obj-m += iot_uart_driver.o

# internal object composition (-y means these are the objects that make up the final module)
iot_uart_driver-y := \
        iot_uart_driver_main.o \
        parser/frame_parser.o \
        crc/crc16.o \
        crc/crc8.o \
        crc/checksum.o \
        crc/validator.o

ifdef ARCH
# If ARCH is defined, we assume we're cross-compiling and use the appropriate KDIR
KDIR := /home/crossBuild/tmp/lib/modules/5.10.65-42-generic/build
else
KDIR := /lib/modules/$(shell uname -r)/build
endif

PWD := $(shell pwd)

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean