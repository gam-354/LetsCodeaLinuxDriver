# 11 - Interrupts (Alternative version)

The purpose of this exercise is to register the module as handler of a specific HW interruption and implement the functionality to handle it when it is triggered. 

Instead of using Gpios, this version will use the PS2 keyboard of the computer to avoid any dependance with other HW. The embedded keyboard of laptops is connected through PS2 interface. If this peripheral is not available in your setup, consider using other devices such as the usb controller or the touchpad.

### Setup

Copy files from exercise **01 - Hello World**. Rename them accordingly so that the module is called `interrupts`.

Add the `interrupt.h` header to the list of `#include` directives

When registering and de-registering from the IRQ control, the ID of the interruption must be passed as argument. The ISA defines a standarized list of IRQ ID's and common devices, among them the keyboard is found with ID 1.

Unfortunately, there is no such list in the Linux Kernel headers (except for ARM architectures). We need to define the ID by ourselves:

```
#define IRQ_ID 1
```

Additionally, a report of the current IRQs and some statistics can be seen in `/proc/interrupts` system file. The first column shows the ID of every interruption.


### Registering the IRQ

In the module initialization, request_irq() function must be called:

```
int error = request_irq(IRQ_ID, myHandler, IRQF_SHARED, "my_kbd_handler", THIS_MODULE);
```

Explanation of the arguments:
* `IRQ_ID`: the ID of the IRQ the module is registering to.
* `myHandler`: name of the function that will handle the interruption. To be defined later
* `IRQF_SHARED`: flag that indicates that this IRQ will be also attended by other drivers. We don't want to substitute the keyboard driver.
* `"my_kbd_handler"`: name of the IRQ handler. Free choice.
* cookie data to be shared with the interrupt handler. For this exercise, no specific data is needed. However, if the interrupt is shared, linux kernel does not accept a NULL pointer here. We can opt to use THIS_MODULE pointer, which is automatically created and filled in with data from this module.

### De-registration

In the exit module callback, just call free_irq() funciton:

```
free_irq(IRQ_ID, THIS_MODULE);
```

### IRQ handling

As stated before, myHandler will be the function that the kernel will call when a registered interruption has been triggered. It will come with the ID of the IRQ (in case this module has been registered to more than one IRQ), and the cookie data. For this exercise, both can be discared.

```
static irqreturn_t myHandler(int irq_no, void * dev_id)
{
   printk("Interrputed with interrupt ID: %d!\n", irq_no);

   return IRQ_NONE;
}
```

It is important to return `IRQ_NONE` so that the original drivers still have the chance to process the interruption.

## Check the results

Load the kernel module from your terminal using `insmod` command. You can check the log traces in `/var/log/syslog`. You may tail this file to see the events in real time:

```
tail -f /var/log/syslog
```

In another terminal, print `/proc/interrupts` file. You should see my_kbd_handler linked to the selected interrupt. In the case of IRQ ID 1, the name should appear at the end of the first line, along with the i8042 driver. 

Now press some keys of the laptop keyboard and check syslog traces. The log trace printed from `myHandler()` function should appear several times. Note that for every key press, 2 interruptions are triggered: one for the key press and another for the key release.


