# Makefile for building the project named iot_uart_driver

obj-m += iot_uart_driver.o

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
