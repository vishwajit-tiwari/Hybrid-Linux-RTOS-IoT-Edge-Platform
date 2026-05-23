# Hybrid Linux–RTOS IoT Edge Platform

M.Tech Project – BITS Pilani WILP

A hybrid embedded IoT edge platform integrating:
- ESP32 running FreeRTOS
- Raspberry Pi 4B running Linux
- Custom Linux UART character driver
- UART telemetry framework with CRC validation
- MQTT-based cloud telemetry pipeline

---

# System Architecture

ESP32 (FreeRTOS)
    ↓ UART Framed Telemetry
Linux UART Character Driver
    ↓
Frame Synchronization Engine
    ↓
CRC / Checksum Validation
    ↓
kfifo Validated Buffer
    ↓
User-space Telemetry Service
    ↓
MQTT Publisher
    ↓
Cloud Dashboard

---

# Current Features

## ESP32 RTOS Side
- FreeRTOS multitasking
- Supervisor-centric runtime architecture
- DS18B20 temperature sensing
- INA219 voltage/current/power sensing
- PIR motion detection
- UART framed telemetry
- Configurable CRC integrity framework
- Task monitoring and graceful degradation

## Linux Kernel Driver
- Custom Linux character device driver
- Dynamic device registration
- kfifo-based buffering
- mutex synchronization
- blocking/non-blocking I/O
- wait queue support
- poll/select asynchronous I/O
- UART integration framework
- modular parser architecture

## Planned Linux-side Features
- Frame synchronization engine
- CRC8 / CRC16 / checksum validation
- validated telemetry buffering
- user-space telemetry service
- MQTT cloud publishing
- runtime diagnostics

---

# Hardware Used

| Component | Purpose |
|---|---|
| ESP32-WROOM | RTOS telemetry node |
| Raspberry Pi 4B (4GB) | Linux edge gateway |
| DS18B20 | Temperature sensing |
| INA219 | Voltage/current/power sensing |
| PIR Sensor | Motion detection |

---

# UART Telemetry Protocol

Example telemetry frame:

```text
<T:30.12,I:0.02,V:0.99,P:0.02,M:0>*5BD7
```

Protocol Features:
- framed UART packets
- configurable integrity modes
- CRC16 / CRC8 / checksum support
- stream synchronization
- packet validation

---

# Linux Driver Features

- Character device driver
- kfifo buffering
- wait queues
- poll/select support
- asynchronous event-driven I/O
- modular telemetry architecture

---

# Current Project Status

## Completed
- ESP32 RTOS telemetry framework
- UART communication validation
- Raspberry Pi UART configuration
- Linux character driver
- kfifo buffering
- poll/select support
- blocking/non-blocking reads
- hardware telemetry validation

## In Progress
- Frame synchronization engine
- CRC validation framework
- telemetry service
- MQTT cloud integration

---

# Repository Structure

```text
hybrid_iot_platform/
│
├── kernel_driver/
├── user_space/
├── scripts/
├── docs/
├── screenshots/
└── README.md
```

---

# Author

Vishwajit Kumar Tiwari

M.Tech – Embedded Systems  
BITS Pilani WILP