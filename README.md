# Hybrid Linux-RTOS IoT Edge Platform

M.Tech Project (BITS Pilani WILP)

## Architecture
ESP32 (FreeRTOS)
    ↓ UART
Linux UART Character Driver
    ↓
User-space Telemetry Service
    ↓
MQTT
    ↓
Cloud Dashboard

## Features
- UART character driver
- kfifo buffering
- configurable CRC framework
- poll/select support
- ESP32 telemetry integration
- MQTT publishing
- fault-tolerant architecture

## Hardware
- ESP32
- Raspberry Pi 4B
- DS18B20
- INA219
- PIR Sensor

## Status
Phase-1:
- Character driver
- kfifo buffering