#!/bin/bash

echo "Configuring UART..."

sudo stty -F /dev/ttyAMA0 115200 raw -echo

echo "Removing old driver..."
sudo rmmod iot_uart_driver 2>/dev/null

echo "Loading driver..."
sudo insmod kernel_driver/iot_uart_driver.ko

echo "Starting telemetry stream..."

sudo cat /dev/iot_uart
