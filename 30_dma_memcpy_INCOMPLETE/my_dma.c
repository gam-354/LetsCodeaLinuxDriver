#include <linux/module.h>
#include <linux/init.h>

#include <linux/completion.h>
#include <linux/slab.h>
#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>

#include <linux/syscalls.h>
#include <linux/fcntl.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Guille");
MODULE_DESCRIPTION("Simple DMA example");

void my_dma_transfer_completed(void * param)
{
   struct completion * cmp = (struct completion * ) param;
   complete(cmp);
}

static int __init myInit(void)
{
   dma_cap_mask_t mask;
   struct dma_chan * chan = NULL;
   struct dma_async_tx_descriptor * chan_desc;
   
   dma_cookie_t cookie;
   dma_addr_t src_addr, dst_addr;

   u8 * src_buf, * dst_buf;

   struct completion cmp;

   int status;

   // int fd;
   // char * device_file_name = "/dev/i2c-0";

   printk("my_dma - init!\n");

   // Configure the DMA capabilities to SLAVE & PRIVATE. Clear the mask first
   dma_cap_zero(mask);  
   dma_cap_set(DMA_SLAVE | DMA_PRIVATE, mask);

   chan = dma_request_channel(mask, NULL, NULL);



   // GUILLE: try to use dma_request_chan( device * dev).
   // Read "DMA engine" guide from google
   // Also read https://www.linuxjournal.com/article/8110?page=0,1
   // to know how to access devices from kernel

   if(!chan || IS_ERR(chan))
   {
      printk("my_dma - Error requesting DMA channel\n");
      return -ENODEV;
   }

   printk("my_dma - channel name: %s\n", chan->name);
   printk("my_dma - channel device name: %s\n", chan->device->dev->init_name);
   //printk("my_dma - channel slave device name: %s\n", chan->slave->init_name);

   // Allocate memory for the buffers
   // The function dma_alloc_coherent function does two special things 
   // apart from allocating the memory
   // - Makes sure that memory can't be cached
   // - It returns the physical address of the memory, not

   src_buf = dma_alloc_coherent(chan->device->dev, 4, &src_addr, GFP_KERNEL);
   dst_buf = dma_alloc_coherent(chan->device->dev, 4, &dst_addr, GFP_KERNEL);

   // Initialize our buffers
   memset(src_buf, 0x12, 1024);
   memset(dst_buf, 0x00, 1024);

   printk("my_dma - Before DMA transfer: src_buf[0] = %x\n", src_buf[0]);
   printk("my_dma - Before DMA transfer: dst_buf[0] = %x\n", dst_buf[0]);

   // Configure the DMA operation
   chan_desc = dmaengine_prep_dma_memcpy(chan, dst_addr, src_addr, 4, DMA_MEM_TO_DEV);

   if (!chan_desc)
   {
      printk("my_dma - Error configuring DMA channel descriptor\n");
      status = -1;
      goto free;
   }

   // Configure the callback
   init_completion(&cmp);
   chan_desc->callback = my_dma_transfer_completed;
   chan_desc->callback_param = &cmp;

   // Fire the DMA transfer
   dma_async_issue_pending(chan);

   if(wait_for_completion_timeout(&cmp, msecs_to_jiffies(3000)) <= 0)
   {
      printk("my_dma - Timeout!\n");
      status = -1;
      goto release;
   }

   // Check the DMA result
   status = dma_async_is_tx_complete(chan, cookie, NULL, NULL);
   if (status == DMA_COMPLETE)
   {
      printk("my_dma - DMA transfer has been completed\n");
      status = 0;
      printk("my_dma - After DMA transfer: src_buf[0] = %x\n", src_buf[0]);
      printk("my_dma - After DMA transfer: dst_buf[0] = %x\n", dst_buf[0]);
   }
   else
   {
      printk("my_dma - Error on DMA transfer\n");

   }

release:
   dmaengine_terminate_all(chan);

free:
   dma_free_coherent(chan->device->dev, 1024, src_buf, src_addr);
   dma_free_coherent(chan->device->dev, 1024, dst_buf, dst_addr);

   dma_release_channel(chan);
   return 0;
}

static void __exit myExit(void)
{
   printk("Nos vamos!\n");
   return;
}

module_init(myInit);
module_exit(myExit);
