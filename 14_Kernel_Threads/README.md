# 14 - Kernel threads

In this exercise, two kernel threads will be launched. Both threads will run the same function with their own variables and periodicity. At the closure of the kernel module, both threads must be deleted

### Preparation

Copy exercise **01 - Hello World** and rename `helloWorld.c` as `kthread.c`. Rename logging and Makefile reference accordingly.

Add the following headers:

```
#include <linux/kthread.h>  // For managing threads
#include <linux/sched.h>
#include <linux/delay.h>    // To call msdelay function
```

### Thread variables and functions

First, declare two global variables for each of the threads. One is a `task_struct` pointer, which will be assigned by the kernel upon the creation of the thread. The other one just holds the thread number, to allow identifying the traces from each thread and to practice passing arguments to the thread function.

```
static struct task_struct * kthread_1;
static struct task_struct * kthread_2;
static int t1 = 1;         // Thread number for thread 1
static int t2 = 2;         // Thread number for thread 2
```

Now let's declare the thread function, that is, the function that the thread will run when launched. Thread functions must return an integer and may receive parameters as void pointers.

In order to have both threads running in parallel and seeing some evolution in them, let's create a while loop that shows the value of a counter, then increments it, and then sleeps for some time. This will be done until the thread is stopped externally:

```
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
```

the `kthread_should_stop()` function returns `false` until the thread is stopped by the kernel. From that moment onwards, the function will return `true`.

### Threads initialization

We will reuse `myInit()` function to create and launch the threads. Thread 1 will be created and launched separately, using `kthread_create()` and `wake_up_process()` respectively. `kthread_create()` takes the function object, the arguments and a thread name as parameters, and it returns the `task_struct` pointer initialized to the thread internal data.

```
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
   (...)
```

Thread 2 will be created and launched in one operation, with the kthread_run() function. It takes the same arguments as kthread_create(), but it also launches the thread after initializing it.

```
   (...)
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
```

### Threads termination

In the `myExit()` function, threads must be stopped using `kthread_stop()` function, passing the `task_struct` pointer as argument.

```
static void __exit myExit(void)
{
   printk("kthread - Stopping both threads and exiting!\n");
   kthread_stop(kthread_1);
   kthread_stop(kthread_2);
   return;
}
```

## Test the results

Build the kernel module and check the kernel logs (available in `syslog` file and `dmesg` output). You should see traces from each of the two threads. Thread 1 should increment the counter twice as fast as Thread 2, since it has a shorter sleep time.

```
[ 1191.610057] kthread - Init threads
[ 1191.610093] kthread - Thread 1 was created and it is running now!
[ 1191.610095] kthread - Thread 1 is executed! Counter val: 0
[ 1191.610109] kthread - Thread 2 was created and it is running now!
[ 1191.610111] kthread - Thread 2 is executed! Counter val: 0
[ 1192.627417] kthread - Thread 1 is executed! Counter val: 1
[ 1193.623507] kthread - Thread 2 is executed! Counter val: 1
[ 1193.651331] kthread - Thread 1 is executed! Counter val: 2
[ 1194.675346] kthread - Thread 1 is executed! Counter val: 3
[ 1195.635484] kthread - Thread 2 is executed! Counter val: 2
[ 1195.699364] kthread - Thread 1 is executed! Counter val: 4
[ 1196.727304] kthread - Thread 1 is executed! Counter val: 5
[ 1197.651395] kthread - Thread 2 is executed! Counter val: 3
[ 1197.747413] kthread - Thread 1 is executed! Counter val: 6
[ 1198.771215] kthread - Thread 1 is executed! Counter val: 7
[ 1199.667171] kthread - Thread 2 is executed! Counter val: 4
[ 1199.795226] kthread - Thread 1 is executed! Counter val: 8
[ 1200.819146] kthread - Thread 1 is executed! Counter val: 9
[ 1201.683268] kthread - Thread 2 is executed! Counter val: 5
[ 1201.843175] kthread - Thread 1 is executed! Counter val: 10
```

In order to stop the threads, remove the kernel module using `rmmod` command. You should see:

```
[ 1222.341530] kthread - Stopping both threads and exiting!
[ 1223.346556] kthread - Thread 1 finished execution!
[ 1223.858620] kthread - Thread 2 finished execution!
```

Depending on the remaining time in each of the `msleep()` calls, threads may take some time to end.

