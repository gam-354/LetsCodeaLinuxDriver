#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>      // To allow issuing ioctl commands
#include <signal.h>

#include "ioctl_commands.h"

int signal_received = 0;

void signalHandler(int sig)
{
    printf("Signal received!\n");
    signal_received = 1;
}

int main()
{
    int fd;

    // Register the signal handler function
    signal(SIGNR, signalHandler);

    printf("PID: %d\n", getpid());

    // Open the device file
    fd = open("/dev/signals", O_WRONLY);
    if (fd == -1)
    {
        printf("Opening was not possible\n");
        return -1;
    }

    // Register app to KM
    if(ioctl(fd, REGISTER_UAPP, NULL) > 0)
    {
        perror("Error registering app");
        close(fd);
        return -1;
    }

    printf("Waiting for signal... \n");
    while(!signal_received)
    {
        sleep(1);
    }

    close(fd);
    return 0;
}

