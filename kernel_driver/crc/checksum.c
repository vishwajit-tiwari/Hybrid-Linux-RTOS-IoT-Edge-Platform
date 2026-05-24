#include <linux/kernel.h>

#include "integrity.h"

uint8_t checksum_compute(const uint8_t *data, int len)
{
    uint8_t sum = 0;
    int i;

    for (i = 0; i < len; i++)
    {
        sum += data[i];
    }

    return sum;
}