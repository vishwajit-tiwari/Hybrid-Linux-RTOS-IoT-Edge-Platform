#include <linux/kernel.h>
#include <linux/string.h>

#include "integrity.h"

#define CRC16_POLY 0x1021

uint16_t crc16_compute(const uint8_t *data, int len)
{
    uint16_t crc = 0xFFFF;

    int i;
    int j;

    for (i = 0; i < len; i++)
    {
        crc ^= (data[i] << 8);

        for (j = 0; j < 8; j++)
        {
            if (crc & 0x8000)
            {
                crc = (crc << 1) ^ CRC16_POLY;
            }
            else
            {
                crc <<= 1;
            }
        }
    }

    return crc;
}