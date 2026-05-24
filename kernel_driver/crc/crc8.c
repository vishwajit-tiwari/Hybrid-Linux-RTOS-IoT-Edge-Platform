#include <linux/kernel.h>

#include "integrity.h"

uint8_t crc8_compute(const uint8_t *data, int len)
{
    uint8_t crc = 0x00;
    int i, j;

    for (i = 0; i < len; i++)
    {
        crc ^= data[i];

        for (j = 0; j < 8; j++)
        {
            if (crc & 0x80)
            {
                crc = (crc << 1) ^ 0x07;
            }
            else
            {
                crc <<= 1;
            }
        }
    }

    return crc;
}