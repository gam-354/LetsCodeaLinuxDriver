#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <string.h>

#include "defs.h"

int main()
{
    struct pollfd my_poll;

    // Open the device file
    int fd = open(DEVICE_FILE_NAME, O_RDONLY);
    if (fd == -1)
    {
        perror("Opening was not possible\n");
        return -1;
    }

    memset(&my_poll, 0, sizeof(my_poll));
    my_poll.fd = fd;
    my_poll.events = POLLIN;

    printf("Polling... \n");
    poll(&my_poll,1,-1);
    printf("Unlocked! \n");

    close(fd);
    return 0;
}

