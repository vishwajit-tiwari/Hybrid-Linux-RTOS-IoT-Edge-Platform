#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define DEVICE_PATH "/dev/iot_uart"
#define BUFFER_SIZE 256

int main()
{
    int fd;

    char buffer[BUFFER_SIZE];

    ssize_t bytes_read;

    printf("Telemetry Service Starting...\n");

    fd = open(DEVICE_PATH, O_RDONLY);

    if (fd < 0)
    {
        perror("Failed to open /dev/iot_uart");
        return -1;
    }

    printf("Connected to %s\n", DEVICE_PATH);

    while (1)
    {
        memset(buffer, 0, sizeof(buffer));

        bytes_read = read(fd, buffer, sizeof(buffer) - 1);

        if (bytes_read > 0)
        {
            buffer[bytes_read] = '\0';

            printf("Telemetry Frame: %s", buffer);

            fflush(stdout);
        }
        else if (bytes_read < 0)
        {
            perror("Read error");
            break;
        }

        usleep(100000);
    }

    close(fd);

    return 0;
}
