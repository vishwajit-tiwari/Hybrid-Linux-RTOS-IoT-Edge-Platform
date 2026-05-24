#include <linux/kernel.h>
#include <linux/string.h>

#include "integrity.h"

int validate_uart_frame(const char *frame)
{
    char payload[128];

    char *star_ptr;

    unsigned int received_crc;
    unsigned int computed_crc;

    int payload_len;

    star_ptr = strchr(frame, '*');

    if (!star_ptr)
    {
        pr_err("crc: invalid frame format\n");
        return 0;
    }

    payload_len = star_ptr - frame;

    strncpy(payload, frame, payload_len);
    payload[payload_len] = '\0';

    sscanf(star_ptr + 1, "%x", &received_crc);

#if CURRENT_CRC_MODE == CRC16_MODE

    computed_crc =
        crc16_compute((const uint8_t *)payload,
                      payload_len);

#elif CURRENT_CRC_MODE == CRC8_MODE

    computed_crc =
        crc8_compute((const uint8_t *)payload,
                     payload_len);

#else

    computed_crc =
        checksum_compute((const uint8_t *)payload,
                         payload_len);

#endif

    if (computed_crc == received_crc)
    {
        pr_info("crc: frame validation success\n");
        return 1;
    }

    pr_err("crc: validation failed computed=0x%X received=0x%X\n",
           computed_crc,
           received_crc);

    return 0;
}