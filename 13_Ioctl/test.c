#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/ioctl.h>      // To allow issuing ioctl commands
#include "ioctl_commands.h"

int main()
{
    int answer;
    struct myStruct test = {3, "Pepe"};

    int dev = open("/dev/dummy", O_WRONLY);
    if (dev == -1)
    {
        printf("Opening was not possible\n");
        return -1;
    }

    ioctl(dev, RD_VALUE, &answer);
    printf("The answer is %d\n", answer);

    // Now try to write the answer to the module:
    answer = 123;
    ioctl(dev, WR_VALUE, &answer);
    
    // Read it back to check
    ioctl(dev, RD_VALUE, &answer);
    printf("The answer now is %d\n", answer);

    // Test greeter command
    ioctl(dev, GREETER, &test);

    printf("Opening successful!\n");

    close(dev);
}