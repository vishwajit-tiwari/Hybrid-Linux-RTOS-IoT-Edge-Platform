#ifndef INTEGRITY_H
#define INTEGRITY_H

#include <linux/types.h>

#define CRC16_MODE     16
#define CRC8_MODE      8
#define CHECKSUM_MODE  0

#define CURRENT_CRC_MODE CRC16_MODE

uint16_t crc16_compute(const uint8_t *data, int len);
uint8_t crc8_compute(const uint8_t *data, int len);
uint8_t checksum_compute(const uint8_t *data, int len);

int validate_uart_frame(const char *frame);

#endif