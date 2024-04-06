# 16 - Poll Callback

This exercise explores the "poll" functionality that a kernel module has. This allows a program running in user space to get blocked for an event that is received in the kernel space.

This is a variation of the official Exercise 16. Here, two user-space executables will be created: one for getting blocked through the poll call, and another to unlock the former using an IOCTL call.

## Preparation

Take exercise "13 - IOCTL" as base. Rename kernel module to `pollCallback` and propagate the change.

Create a header file called defs.h. It will contain two constants to be shared across the executables. One is the IOCTL Unlocking command ID, and the other one is the device file name.

```
#define CMD_UNLOCK _IO('R', 'g')
#define DEVICE_FILE_NAME "/dev/poll"
```

## Main code

Ensure the module includes the following headers:

```
#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/poll.h>
#include <linux/wait.h>

#include "defs.h"
```

Declare a couple of variables to manage the wait. One is just a flag (simulating the arrival of an IRQ or any other external event) and the other one is a wait_queue_head_t structure, which is the shared object to be used for the wait.

```
static int irq_ready = 0;
static wait_queue_head_t waitqueue;
```

#### Module initialization

First, initialize the `waitqueue` structure.

```
init_waitqueue_head(&waitqueue);
```

Then, add the code to register the device, as done in many previous exercises:

```
// Register the device number for a new character device
int retVal = register_chrdev(MY_MAJOR, "LKM_poll", &fops);

// If 0, this means that the slot was free and this is the first device registeredin it.
if(retVal == 0)
{
   printk("signals - registered Device number Major: %d, Minor, %d\n", MY_MAJOR, 0);
}

// If positive, there is already one device registered. Check Major and Minorreturned values
// by shifting some bits:
else if(retVal > 0)
{
   int major = retVal >> 20;
   int minor = retVal & 0xFFFFF;
   printk("signals - registered Device number Major: %d, Minor, %d\n", major, minor);
}

// If negative, then something went wrong
else
{
   printk("signals - Could not register device number!\n");
   return -1;
}
```

Finally, make sure that the myInit function returns 0.

Also remember to add the `.poll` capability in the `file_operations` structure.

```
static struct file_operations fops = {
   .owner = THIS_MODULE,
   .unlocked_ioctl = my_ioctl,    // name of ioctl function
   .poll = my_poll,
   .release = my_close
};
```

#### Module exit

Here the module just has to unregister the char device.

```
unregister_chrdev(MY_MAJOR, "LKM_poll");
```

#### Poll callback

The poll callback function takes a `file` pointer and a `poll_table` pointer. Both of them will be passed-through directly to `poll_wait()` kernel function, along with our `waitqueue` object.

```
// Poll callback:
static unsigned int my_poll(struct file * file, poll_table * wait)
{
   poll_wait(file, &waitqueue, wait);
   if (irq_ready == 1)
   {
      irq_ready = 0;
      return POLLIN;
   }
   return 0;
}
```

#### IOCTL management

Define the ioctl function. This time, it only dispatches the `CMD_UNLOCK` command. When this request arrives, the irq_ready flag is raised and the wake_up() function is called to unlock the waiting threads.

```
static long int my_ioctl(struct file * file, unsigned cmd, unsigned long arg) 
{
   if (cmd == CMD_UNLOCK)
   {
      printk("poll - Waking up polling processes\n");
      irq_ready = 1;
      wake_up(&waitqueue);
   }
   return 0;
}
```


## Test applications

### Poll application

This program will open the device file and call poll() linux function (ensure you have `poll.h` header included). This call will reach `my_poll()` function in the kernel module and will be blocked until the kernel module returns from `poll_wait()`.

```
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
```

### Unlock application

This program will also open the device file and send an `ioctl()` call with the `CMD_UNLOCK` argument. This command will make the kernel wake up the waiting queue. Ensure `defs.h` , `ioctl.h` and `fcntl.h` headers are loaded.

```
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
```

## Test the results

Build and load the kernel module and check the kernel logs (available in `syslog` file and `dmesg` output).

Now create the device file from the terminal with the `mknod` command, passing the Major and Minor numbers:

```
sudo mknod /dev/signals c 91 0
```

Remember to give 666 permissions to `/dev/signals` file.

Build the test applications with gcc.

First, run `test_poll` application. This process will lock the terminal waiting for the signal.

Then, open a different terminal and run `test_unlock` application. This app will end immediately. Back in the first terminal, `test_poll` should have ended with the corresponding printed message.
