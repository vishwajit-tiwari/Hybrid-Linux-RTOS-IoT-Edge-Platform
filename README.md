# Hybrid Linux–RTOS IoT Edge Platform

![Platform](https://img.shields.io/badge/Platform-ESP32%20%7C%20RaspberryPi-blue)
![Language](https://img.shields.io/badge/Language-C-green)
![Kernel](https://img.shields.io/badge/Linux-Kernel%20Driver-red)
![RTOS](https://img.shields.io/badge/RTOS-FreeRTOS-orange)

M.Tech Project – BITS Pilani WILP

A hybrid embedded IoT edge platform integrating:
- ESP32 running FreeRTOS
- Raspberry Pi 4B running Linux
- Custom Linux UART character driver
- UART telemetry framework with CRC validation
- MQTT-based cloud telemetry pipeline

---

# System Architecture

```text
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
```

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
- user-space telemetry service
- MQTT cloud publishing
- telemetry dashboard visualization
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
- CRC16-CCITT integrity validation
- cross-platform Linux/ESP32 CRC compatibility

---

# Linux Driver Features

- Character device driver
- kfifo buffering
- wait queues
- poll/select support
- asynchronous event-driven I/O
- modular telemetry architecture
- modular CRC validation framework
- CRC16 telemetry validation
- CRC8 validation support
- checksum validation mode
- configurable integrity framework
- validated telemetry-only buffering
- UART stream accumulation handling
- fragmented frame recovery
- stream-oriented telemetry parsing
- producer-consumer telemetry buffering model

---

# Current Linux Driver Runtime Pipeline

Current implemented Linux telemetry flow:

```text
ESP32 UART Stream
        ↓
UART RX Kernel Thread
        ↓
Frame Synchronization Parser
        ↓
Validated Telemetry Frame
        ↓
kfifo Buffer
        ↓
Character Device (/dev/iot_uart)
        ↓
User-space Reader
```

Example runtime telemetry:

```text
<T:36.00,I:0.02,V:0.96,P:0.02,M:0>*8BEC
```

---

# Current Project Status

## Completed
- ESP32 RTOS telemetry framework
- UART communication validation
- Raspberry Pi UART configuration
- Linux character driver
- UART RX kernel thread
- frame synchronization parser
- kfifo validated buffering
- poll/select support
- blocking/non-blocking reads
- safe module unload handling
- poll/select-based asynchronous I/O
- hardware telemetry validation
- UART hardware configuration validation
- verified ESP32 ↔ Raspberry Pi telemetry streaming
- CRC16 telemetry integrity verification
- live framed telemetry reception over ttyAMA0


## In Progress
- user-space telemetry service
- MQTT cloud integration
- telemetry dashboard visualization

---

# Current Runtime Validation

- CRC16 telemetry validation fully operational
- stream-based UART frame synchronization implemented
- validated telemetry buffering pipeline operational
- real-time ESP32 → Linux telemetry reception verified
- FIFO overflow handling implemented
- UART raw-mode runtime stabilization

---

# Engineering Challenges Solved

- UART stream fragmentation handling
- partial frame reconstruction
- cross-platform CRC16 compatibility
- FIFO overflow protection
- blocking vs non-blocking synchronization
- UART raw mode stabilization
- safe kernel thread termination

---

# Demo Execution

Automated demo startup script:

```bash
./scripts/start_demo.sh
```
Actual demo path:

```md
Run from project root:

```bash
./scripts/start_demo.sh
```

The script automatically:

- configures UART raw mode
- reloads kernel driver
- starts telemetry streaming
- displays validated telemetry frames

Example runtime output:

```text
<T:30.81,I:0.01,V:0.97,P:0.01,M:0>*606C
```

---

# Screenshots

- ESP32 telemetry transmission
- Linux driver runtime logs
- validated telemetry stream
- MQTT telemetry publishing

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

LinkedIn: [Vishwajit Kumar Tiwari](https://www.linkedin.com/in/vishwajit-tiwari/)
GitHub: [github.com/vishwajit-tiwari](https://github.com/vishwajit-tiwari)