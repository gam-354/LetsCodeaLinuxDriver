# 18 - Procfs entry

In the early days of Linux Kernel, the only way to acess information about the processes was to read a structures living in the kernel memory, but it required to read raw data with root permissions, which was not either robust or secure.

That's why the `proc file system` was created. It is a standarized way of accessing information about kernel modules and modify some of their settings.

In this exercise we will create the "hello" directory inside `/procfs` path, containing the "dummy" file. We will also define two callbacks for the read and write operations on this file.

**NOTE**: due to recent kernel changes, the code in this version of the exercise will not match exactly the version in the Youtube tutorial.

## Preparation

Copy exercise "**01 - Hello World**" and rename the main C file to procfs.c.

Ensure the following headers are included:

```
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
```

## Main code

Create a couple of global variables related to the procfs folder and file parameters. Both of them are `proc_dir_entry` structure pointers.

```
// Global variables for procfs folder and file
static struct proc_dir_entry * proc_folder;
static struct proc_dir_entry * proc_file;
```

### Read/write callbacks:

The callbacks will be very similar to the ones that we are used to. They come with a struct file pointer (that we will disregard for this exercise), a buffer, an integer to control the buffer, and an offset (which we will ignore too).

In the read callback, called `driver_read()`, we will just insert a predefined text into the buffer and return the amount of bytes copied:

```
static ssize_t driver_read(struct file * File, char * user_buffer, size_t count, loff_t * offset)
{
   char text[] = "Hello from a procfs file\n";
   int to_copy, not_copied, delta;

   // 1. Get the amount of data to copy
   to_copy = min(count, sizeof(text));

   // 2. Copy the data to the user
   not_copied = copy_to_user(user_buffer, text, to_copy);

   // 3. Calculate how much data it has copied
   delta = to_copy - not_copied;
   return delta;
}
```

Note: it is important to insert a newline character at the end of the text, for reasons that will be understood later.

In the write callback, we will just print the buffer contents into a kernel log trace.

```
static ssize_t driver_write(struct file * File, const char * user_buffer, size_t count, loff_t * offset)
{
   char buffer[256];
   int to_copy, not_copied, written;

   // 1. Get the amount of data to copy
   to_copy = min(count, sizeof(buffer));

   // 2. Copy the data to the user
   not_copied = copy_from_user(buffer, user_buffer, to_copy);

   // 3. Print in kernel log:
   printk("procfs - You have writen: %s", buffer);
   
   // 4. Return how much data has been written
   written = to_copy - not_copied;
   return written;
}
```

### Proc operations structure

Similarly to the `file_operations` structure, we will define a set of properties that will apply to the procfs entry. In this case, we just have to set the addresses of the callbacks:

```
static struct proc_ops pops = {
   .proc_read = driver_read,
   .proc_write = driver_write
};
```


### Initialization

In the myInit() function, call `proc_mkdir()` function to create the `hello` directory:

```
proc_folder = proc_mkdir("hello", NULL);
if(proc_folder == NULL)
{
   printk("procfs - Error creating 'hello' file\n");
   return -ENOMEM;
}
```

Now call `proc_create()` to create the `dummy` file. This call must pass the name of the file, the permissions code, the pointer to the `proc_folder` previously created and the pointer to the `proc_ops` structure.


## Test

After building with `make`, the module is ready to be loaded into the kernel:

```
sudo insmod procfs.ko
```

Now, /proc/ should show the folder `hello` with the corresponding permissions. Insider `hello`, the `dummy` file should be present.

A write operation can be triggered by simply echoing a string to the file:

```
echo "good morning!" > /proc/hello/dummy

```
The kernel traces should report it:

```
[ 4686.405025] procfs - You have writen: good morning
```

A read operation can be triggered by performing a `cat` of the file. However, since `cat` command reads until an EOF (end of file) character is found, we need to stop it at some point. With `head -n 1`, it will stop after finding one newline character, which we added intentionally in the text buffer.

```
$> cat /proc/hello/dummy | head -n 1
Hello from a procfs file
```

When removing the kernel module with `rmmod`, both the `hello` directory and `dummy` file will have disappeared.

