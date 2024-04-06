#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Guille");
MODULE_DESCRIPTION("__");

// Global variables for procfs folder and file
static struct proc_dir_entry * proc_folder;
static struct proc_dir_entry * proc_file;


/**
 * @brief Read data out of the buffer. In this case, we will return a predefined text
 */
static ssize_t driver_read(struct file * File, char * user_buffer, size_t count, loff_t * offset)
{
   char text[] = "Hello from a procfs file\n";
   int to_copy, not_copied, delta;

   // 1. Get the amount of data to copy, which will be the minimum
   // between the amount of bytes requested and the amount of bytes
   // stored in the buffer (indicated by the pointer value)
   to_copy = min(count, sizeof(text));

   // 2. Copy the data to the user
   not_copied = copy_to_user(user_buffer, text, to_copy);

   // 3. Calculate how much data it has copied
   delta = to_copy - not_copied;
   return delta;
}

/**
 * @brief Write data to buffer
 */
static ssize_t driver_write(struct file * File, const char * user_buffer, size_t count, loff_t * offset)
{
   char buffer[256];
   int to_copy, not_copied, written;

   // 1. Get the amount of data to copy, which will be the minimum
   // between the amount of bytes requested and the size of the buffer
   to_copy = min(count, sizeof(buffer));

   // 2. Copy the data to the user
   not_copied = copy_from_user(buffer, user_buffer, to_copy);

   // 3. Print in kernel log:
   printk("procfs - You have writen: %s", buffer);
   
   // 4. Return how much data has been written
   written = to_copy - not_copied;
   return written;
}

static struct proc_ops pops = {
   .proc_read = driver_read,
   .proc_write = driver_write
};

/**
 * @brief function called when the module is loaded into the kernel
 */

static int __init myInit(void)
{
   printk("procfs - Hello mundo!\n");

   proc_folder = proc_mkdir("hello", NULL);  // This will create the folder hello in /proc/
   if(proc_folder == NULL)
   {
      printk("procfs - Error creating 'hello' file\n");
      return -ENOMEM;
   }

   // This will create the file "dummy" in /proc/hello/
   proc_file = proc_create("dummy", 0666, proc_folder, &pops);
   if(proc_file == NULL)
   {
      printk("procfs - Error creating 'dummy' file\n");
      proc_remove(proc_folder);
      return -ENOMEM;
   }

   printk("procfs - Created '/proc/hello/dummy' file successfully\n");

   return 0;
}

/**
 * @brief function called when the module is unloaded from the kernel
 */
static void __exit myExit(void)
{
   printk("procfs - Bye bye!\n");
   proc_remove(proc_file);
   proc_remove(proc_folder);
   return;
}

module_init(myInit);
module_exit(myExit);





