#!/bin/bash

echo "====================================="
echo " Hybrid Linux-RTOS IoT Demo Startup "
echo "====================================="

echo "[1] Stopping old services..."

sudo pkill -f telemetry_service 2>/dev/null
sudo pkill -f node-red 2>/dev/null

sleep 2

echo "[2] Removing old driver..."

sudo rmmod iot_uart_driver 2>/dev/null

sleep 1

echo "[3] Configuring UART..."

sudo stty -F /dev/ttyAMA0 115200 raw -echo

echo "[4] Loading driver..."

if lsmod | grep -q iot_uart_driver
then
    echo "Driver already loaded"
else
    sudo insmod kernel_driver/iot_uart_driver.ko
fi

sleep 2

echo "[5] Starting Node-RED..."

if pgrep -x "node-red" > /dev/null
then
    echo "Node-RED already running"
else
    nohup node-red > node_red.log 2>&1 &
fi

sleep 5

echo "[6] Starting telemetry service..."

cd user_space

sudo ./telemetry_service