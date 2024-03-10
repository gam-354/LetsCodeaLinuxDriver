#include <linux/module.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Guille");
MODULE_DESCRIPTION("A simple example for threads in a LKM");

// Global variables for the threads:
static struct task_struct * kthread_1;
static struct task_struct * kthread_2;
static int t1 = 1;         // Thread number for thread 1
static int t2 = 2;         // Thread number for thread 2

// Function that will be executed by the thread
// Args must be passed as void pointers
int thread_function(void * data)
{
   unsigned int i = 0;           // counter
   int t_nr = *(int *) data;     // Thread number

   // Working loop:
   while(!kthread_should_stop())
   {
      // Do something with the counter and print it
      printk("kthread - Thread %d is executed! Counter val: %d\n", t_nr, i++);
      // Delay time depending on thread number
      msleep(t_nr * 1000);
   }

   printk("kthread - Thread %d finished execution!\n", t_nr);
   return 0;
}

static int __init myInit(void)
{
   printk("kthread - Init threads\n");

   // Start Thread 1:

   kthread_1 = kthread_create(thread_function, &t1, "kthread_1");
   if(kthread_1 != NULL)
   {
      // Start the thread:
      wake_up_process(kthread_1);
      printk("kthread - Thread 1 was created and it is running now!\n");
   }
   else
   {
      printk("kthread - Thread 1 could not be created!\n");
      return -1;
   }

   // Start Thread 2:
   // A different way of creating + launching threads:
   kthread_2 = kthread_run(thread_function, &t2, "kthread_2");
   if(kthread_2 != NULL)
   {
      printk("kthread - Thread 2 was created and it is running now!\n");
   }
   else
   {
      printk("kthread - Thread 2 could not be created!\n");
      // Also stop thread 1
      kthread_stop(kthread_1);
      return -1;
   }

   return 0;
}

static void __exit myExit(void)
{
   printk("kthread - Stopping both threads and exiting!\n");
   kthread_stop(kthread_1);
   kthread_stop(kthread_2);
   return;
}

module_init(myInit);
module_exit(myExit);
