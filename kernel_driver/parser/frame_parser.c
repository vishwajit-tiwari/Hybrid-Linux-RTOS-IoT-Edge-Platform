#include <linux/kernel.h>
#include <linux/string.h>

#include "frame_parser.h"

static char frame_buffer[FRAME_MAX_SIZE];

static int frame_index = 0;
static int frame_started = 0;

int parse_uart_stream(const char *input,
                      int len,
                      char *frame_out)
{
    int i;

    for (i = 0; i < len; i++)
    {
        char ch = input[i];

        /*
         * Detect start of frame
         */
        if (ch == '<')
        {
            frame_started = 1;
            frame_index = 0;

            frame_buffer[frame_index++] = ch;

            continue;
        }

        /*
         * Ignore garbage before frame
         */
        if (!frame_started)
        {
            continue;
        }

        /*
         * Prevent frame overflow
         */
        if (frame_index >= FRAME_MAX_SIZE - 1)
        {
            pr_err("parser: frame overflow\n");

            frame_started = 0;
            frame_index = 0;

            return 0;
        }

        frame_buffer[frame_index++] = ch;

        /*
         * Frame complete on newline
         */
        if (ch == '\n')
        {
            frame_buffer[frame_index] = '\0';

            strcpy(frame_out,
                   frame_buffer);

            frame_started = 0;
            frame_index = 0;

            pr_info("parser: extracted frame: %s",
                    frame_out);

            return 1;
        }
    }

    return 0;
}