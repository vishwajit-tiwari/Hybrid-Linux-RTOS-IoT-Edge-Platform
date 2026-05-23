#ifndef FRAME_PARSER_H
#define FRAME_PARSER_H

#define FRAME_MAX_SIZE 128

int parse_uart_stream(const char *input, int len, char *frame_out);

#endif