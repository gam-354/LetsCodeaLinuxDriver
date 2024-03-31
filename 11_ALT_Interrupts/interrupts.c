#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Guille");
MODULE_DESCRIPTION("Simple example of interrupt handling");

// Keyboard always uses IRQ ID 1, according to ISA list
#define IRQ_ID 1

// Interruption handler
static irqreturn_t myHandler(int irq_no, void * dev_id)
{
   printk("Interrputed with interrupt ID: %d!\n", irq_no);

   // Return IRQ_NONE so that the original drivers still have
   // the chance to process the interruption.
   return IRQ_NONE;
}

static int __init myInit(void)
{
   printk("interrupts - initializing\n");

   // Request with IRQF_SHARED to indicate that this IRQ
   // is shared with other drivers. We don't want exclusivity
   int error = request_irq(IRQ_ID, myHandler, IRQF_SHARED, "my_kbd_handler", THIS_MODULE);

   return error;     // 0 if success
}

static void __exit myExit(void)
{
   printk("interrupts - exiting!\n");

   free_irq(IRQ_ID, THIS_MODULE);

   return;
}

module_init(myInit);
module_exit(myExit);
