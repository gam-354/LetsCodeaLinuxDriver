#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>      // To allow issuing ioctl commands

#include "defs.h"

int main()
{
    // Open the device file
    int fd = open(DEVICE_FILE_NAME, O_WRONLY);
    if (fd == -1)
    {
        printf("Opening was not possible\n");
        return -1;
    }

    // Send the unlocking command to the KM
    if(ioctl(fd, CMD_UNLOCK, NULL) > 0)
    {
        perror("Error unlocking");
        close(fd);
        return -1;
    }

    printf("Unlock command sent\n");

    close(fd);
    return 0;
}

