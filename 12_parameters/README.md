# 12 - Passing parameters to kernel modules

Copy directory of exercise No.1 as we are going to use it as a base. 

Rename `helloworld.c` as `params.c`, and propagate this change in the Makefile, updating the value to be added to obj-m variable. You may also want to update all the references inside the log traces.

#### Declare the variables holding the parameters

```
static unsigned int gpio_id = 12;
static char * device_name = "testdevice";

```

#### Declare the parameters to the kernel

```
module_param(gpio_id, uint, S_IRUGO);
module_param(device_name, charp, S_IRUGO);

```
A description can also be added using the `MODULE_PARAM_DESC` macro:

```
MODULE_PARM_DESC(gpio_id, "ID of the gpio to use");
MODULE_PARM_DESC(device_name, "Device name to use");

```

#### Print the parameter values on initialization

Parameters are automatically stored in the variables, which can be accessed from the time of the initialization of the module

```
static int __init myInit(void)
{
   printk("Hello mundo!\n");
   printk("Gpio ID: %u\n", gpio_id);
   printk("Device name: %s\n", device_name);
   return 0;
}
```


## Test

After building with `make`, the module is ready to be loaded into the kernel:

```
sudo insmod params.ko
```

With the command above, parameters will have their default values as defined in the .c file. You can check it in the kernel log:

```
kernel: [ 1082.449343] Hello mundo!
kernel: [ 1082.449346] Gpio ID: 12
kernel: [ 1082.449347] Device name: testdevice
```

To test the parameters functionality, first unload the module. Then, load it again with the following syntax:

```
sudo insmod params.ko device_name="pepe" gpio_id=5
```

This time, parameters are passed towards the module:
```
kernel: [ 1444.154892] Hello mundo!
kernel: [ 1444.154902] Gpio ID: 5
kernel: [ 1444.154903] Device name: pepe
```

#### Detailed module info

The `modinfo` command shows some info about the module that is passed as an argument:

```
$ modinfo params.ko
filename:       /home/gam/Documents/LetsCodeALinuxDriver/guille/12_parameters/params.ko
description:    Simple linux kernel module to demonstrate the use of parameters
author:         Guille
license:        GPL
srcversion:     B1CEE18FD79CAECE917D1EA
depends:        
retpoline:      Y
name:           params
vermagic:       6.5.0-21-generic SMP preempt mod_unload modversions 
parm:           gpio_id:ID of the gpio to use (uint)
parm:           device_name:Device name to use (charp)
```



