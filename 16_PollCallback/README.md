# 15 - Kernel signals to user space

This is a variation of the Chapter 15 in the official Let's Code a Linux Driver project. There, the signal is sent whenever an interruption is triggered by a physical button in the Raspberry device. That requires (obviously) having the corresponding HW. In this version of the exercise, however, the kernel module will send signals by itself using a thread, instead of listening for an interruption.

## Preparation

This exercise is a merge of **14 - Kernel Threads** and **13 - Ioctl**. Any of the source files of them is valid as a code base. At the end of the exercise, we need to have the following:

* **signals.c** -> The file with the source code of the kernel module
* **ioctl_commands.h** -> Header with the signal number and the ioctl command ID
* **test.c** -> A simple C application to listen for kernel space signals

## Main code

This kernel module has to implement the following functionalities:

* 1 thread creation/destruction
* Thread function
* A function to send the signal to user space
* An IOCTL dispatching function to register the client from user space
* Character device registration and de-registration
* Device release function to free resources after client is de-registered

#### Module initialization

Add the code to register the device, as done in many previous exercises:

```
// Register the device number for a new character device
int retVal = register_chrdev(MY_MAJOR, "LKM_signals", &fops);

// retval contains some info encoded.
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

Also add the code to create and launch the thread:

```
kthread_1 = kthread_create(thread_function, &sleep_time, "kthread_1");
if(kthread_1 != NULL)
{
   // Start the thread:
   wake_up_process(kthread_1);
   printk("signals - Thread was created and it is running now!\n");
}
else
{
   printk("signals - Thread could not be created!\n");
   return -1;
}
```

Finally, make sure that the myInit function returns 0.

#### Module exit

Here the module has to stop the thread and unregister the char device.

```
if(kthread_1 != NULL)
{
   kthread_stop(kthread_1);
}
unregister_chrdev(MY_MAJOR, "LKM_signals");
```

#### IOCTL management

Ensure the following headers are included:

```
#include <linux/ioctl.h>
#include "ioctl_commands.h"
```

Open `ioctl_commands.h` and define the two shared constants between the kernel module and the test application:

```
// The kernel will generate a unique magic number for the command
#define REGISTER_UAPP _IO('R', 'g')

// Signal sending parameters
#define SIGNR 44
```

Back in the source code, declare a global variable to hold the details of the registered client:

```
static struct task_struct * task = NULL;

```
Define the ioctl function. This time, it only dispatches the `REGISTER_UAPP` command:

```
static long int my_ioctl(struct file * file, unsigned cmd, unsigned long arg) 
{
   if (cmd == REGISTER_UAPP)
   {
      task = get_current();
      printk("signals - Userspace app with PID %d is registered\n", task->pid);
   }
   return 0;
}
```

The `get_current()` call returns the information of the caller, which is the test application from user space. This information comes in a task_struct object that will be stored globally so that the thread also has access to it. The assignation of this data to `task` variable can be considered the internal signal to the thread to let it know that it is now ready to send the signal to the client.

#### Client de-registration

As explained before, the device file closure callbck (`my_close` in this exercise) will be used to manage the de-registration of the user space client. This means that the information in `task` is now obsolete and must be cleaned out.

```
static int my_close(struct inode * device_file, struct file * instance) 
{
   printk("signals - close was called!\n");
   if (task != NULL)
   {
      task = NULL;
   }
   return 0;
}
```


#### Threads

Ensure the following headers are included:

```
#include <linux/kthread.h>  // For managing threads
#include <linux/sched.h>
#include <linux/delay.h>    // To call msdelay function
```

Define the global variables for the thread: one `task_struct` object and an integer to store the sleeping time of the thread before sending the signal:

```
static struct task_struct * kthread_1;
static int sleep_time = 5;
```

The reason for this sleep is to add some delay between the registration of the client and the signal sending, somehow emulating the time that the operator would have spent until pressing the physical button.

The thread will loop indefinitely until it is killed by the kernel module exit function. Within those loops, only when the `task` variable holds data, the thread will call `send_signal()` to actually send the signal.

```
int thread_function(void * data)
{
   int sleep_time_secs = *(int *)data;

   // Do something with the counter and print it
   printk("signals - Thread function! Sending signal in %d seconds...\n", sleep_time_secs);

   // Working loop:
   while(!kthread_should_stop())
   {
      // Delay time depending on thread number
      msleep(sleep_time_secs * 1000);

      // Send the signal
      if (task != NULL)
      {
         send_signal(task);
      }
   }

   printk("signals - Thread finished execution!\n");
   return 0;
}
```

#### Sending the signal

Finally, the send_signal function will create a `siginfo` struct object to set a couple of parameters, and will call send_sig_info kernel function, passing both the `siginfo` and `task` data as arguments:

```
void send_signal(struct task_struct * task)
{
   struct siginfo info;
   memset(&info, 0, sizeof(info));

   info.si_signo = SIGNR;
   info.si_code = SI_QUEUE;

   if(send_sig_info(SIGNR, (struct kernel_siginfo *)&info, task) < 0)
   {
      printk("signals - error sending signal\n");
   }
}
```

#### File operations

Remember to update the `fops` struct object accordingly:

```
static struct file_operations fops = {
   .owner = THIS_MODULE,
   .release = my_close,
   .unlocked_ioctl = my_ioctl    // name of ioctl function
};
```


## Test application

test.c file should include the following headers:

```
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>      // To allow issuing ioctl commands
#include <signal.h>

#include "ioctl_commands.h"
```

This application needs to:

1. Register a signal handler function
2. Open the device file and send the `REGISTER_UAPP` command via an ioctl call
3. Wait to receive the signal from the kernel space

Let's declare a simple global variable to flag the reception of the signal:

```
int signal_received = 0;
```
The signal handling function will raise this flag when called.

```
void signalHandler(int sig)
{
    printf("Signal received!\n");
    signal_received = 1;
}
```

The main() function needs to do the three actions described above:

```
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
```

Note that the PID of the process is printed in both the test application and the kernel module. This way we can match the info of the client in both sides.


## Test the results

Build and load the kernel module and check the kernel logs (available in `syslog` file and `dmesg` output).

Now create the device file from the terminal with the `mknod` command, passing the Major and Minor numbers:

```
sudo mknod /dev/signals c 91 0
```
Remember to give 666 permissions to `/dev/signals` file.

Build the test application with gcc:

```
gcc test.c -o test
```

Run the application. In that moment, kernel traces will show that the client has registered:

```
[ 1487.576633] signals - Userspace app with PID 5245 is registered
```

Within a period between 0 and `sleep_time` + 1, you should see the application print the message acknowleding the signal:

```
Signal received!
```

You may stop and relaunch the test app as many times as you want. The kernel will manage the subcriptions and it will send the signal the new processes. Note that with this implementation, the kernel module can only handle one client at the same time.



