#include <linux/module.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>

#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/sched/signal.h>     // For signal sending
#include <linux/ioctl.h>

#include "ioctl_commands.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Guille");
MODULE_DESCRIPTION("A simple example for sending signals from a LKM to user space");

// Global variables for the threads:
static struct task_struct * kthread_1;
static int sleep_time = 5;

// Global variables and defines for userspace app registration
static struct task_struct * task = NULL;

#define MY_MAJOR 91     // Free device number. Check list in cat /proc/devices


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

// Function that will be executed by the thread
// Args must be passed as void pointers
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

// IOCTL function for registering the UserSpace app to the kernel module 
static long int my_ioctl(struct file * file, unsigned cmd, unsigned long arg) 
{
   if (cmd == REGISTER_UAPP)
   {
      task = get_current();
      printk("signals - Userspace app with PID %d is registered\n", task->pid);
   }
   return 0;
}

/**
 * @brief function called when the device file is closed
 */
static int my_close(struct inode * device_file, struct file * instance) 
{
   printk("signals - close was called!\n");
   if (task != NULL)
   {
      task = NULL;
   }
   return 0;
}



static struct file_operations fops = {
   .owner = THIS_MODULE,
   .release = my_close,
   .unlocked_ioctl = my_ioctl    // name of ioctl function
};

static int __init myInit(void)
{
   // Register the device number for a new character device
   int retVal = register_chrdev(MY_MAJOR, "LKM_signals", &fops);

   // retval contains some info encoded.

   // If 0, this means that the slot was free and this is the first device registered in it.
   if(retVal == 0)
   {
      printk("signals - registered Device number Major: %d, Minor, %d\n", MY_MAJOR, 0);
   }
   // If positive, there is already one device registered. Check Major and Minor returned values
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

   printk("signals - Init threads\n");

   // Start Thread 1:

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

   return 0;
}

static void __exit myExit(void)
{
   printk("signals - Stopping thread and exiting!\n");
   if(kthread_1 != NULL)
   {
      kthread_stop(kthread_1);
   }
   unregister_chrdev(MY_MAJOR, "LKM_signals");
   return;
}

module_init(myInit);
module_exit(myExit);
