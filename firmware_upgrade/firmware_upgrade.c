#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/earlysuspend.h>

#include <asm/atomic.h>
#include <asm/uaccess.h>

#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/completion.h>

#include <linux/firmware.h>

#define FIRMWARE_MISC_MAGIC 'F'
#define FIRMWARE_MISC_UPGRADE   _IOW(FIRMWARE_MISC_MAGIC,1,uint32_t)

#define KERN_P  "<1>"

static struct miscdevice firmware_misc_device ;

struct dl_image {
    u32 magic;
    u32 id;
    u32 size;
    u32 data[0];
} *image;

static char * firmware_file_name = NULL;

static int firmware_misc_open(struct inode *inode, struct file *file)
{
	printk(KERN_P,"huyanwei debug firmware_misc_open()\n");
    return nonseekable_open(inode, file);
}

static int firmware_misc_release(struct inode *inode, struct file *file)
{
	printk(KERN_P,"huyanwei debug firmware_misc_release()\n");
    return 0;
}

static long firmware_misc_unlocked_ioctl(struct file *file, unsigned int cmd,
                               unsigned long arg)
{
	const struct firmware *fw_entry;
    void __user *data;

    long err = 0;

    if (_IOC_DIR(cmd) & _IOC_READ)
    {
        err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
    }
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
    {
        err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
    }

    if (err)
    {
        printk(KERN_P, "access error: %08X, (%2d, %2d)\n", cmd, _IOC_DIR(cmd), _IOC_SIZE(cmd));
        return -EFAULT;
    }

    switch (cmd)
    {
        case FIRMWARE_MISC_UPGRADE:

            data = (void __user *) arg;

			printk(KERN_P,"huyanwei debug:firmware_misc_release(FIRMWARE_MISC_UPGRADE)\n");

            if (data == NULL)
            {
                err = -EINVAL;
                break;
            }
			
			#if 1
			memcpy(firmware_file_name,"WMT.cfg",strlen("WMT.cfg"));
			#else
            if (copy_from_user(firmware_file_name,data,(strlen(data)>255)?(255):(strlen(data))))
            {
                err = -EFAULT;
                break;
            }
			#endif

			printk(KERN_P,"huyanwei debug:firmware_file_name=%s\n",firmware_file_name);
		
			//err = request_firmware(&fw_entry,firmware_file_name,firmware_misc_device.dev);
			err = request_firmware(&fw_entry,firmware_file_name,firmware_misc_device.this_device);

			#if 0
			if(fw_entry->size <  sizeof(struct dl_image))
			{
				printk(KERN_P,"firmware size too small. load firmware incompleted!\n");
			}
			#endif

			image = (struct dl_image*)fw_entry->data;
		
			printk(KERN_P,"huyanwei debug:firmware_data=%s\n",image);

			release_firmware(fw_entry);

			break;
		default:
            printk(KERN_P,"huyanwei debug:unknown ioctl command: 0x%08x\n", cmd);
            err = -ENOIOCTLCMD;
			break;
	}
    return err;
}

static struct file_operations firmware_misc_fops =
{
//  .owner 			= THIS_MODULE,
    .open 			= firmware_misc_open,
    .release 		= firmware_misc_release,
    .unlocked_ioctl = firmware_misc_unlocked_ioctl,
};

static struct miscdevice firmware_misc_device =
{
    .minor = MISC_DYNAMIC_MINOR,
    .name = "firmware_misc_dev",
    .fops = &firmware_misc_fops,
};

int __init firmware_init(void)
{
	printk(KERN_P,"huyanwei deug:firmware_init()\n");

	firmware_file_name = kzalloc(256,GFP_KERNEL);
    if (misc_register(&firmware_misc_device))
    {
	    printk("firmware_misc_device register failed\n");
    }
}

/* should never be called */
static void __exit firmware_exit(void)
{
	printk(KERN_P,"huyanwei deug:firmware_exit()\n");

	misc_deregister(&firmware_misc_device);

	if(firmware_file_name)
	{
		kfree(firmware_file_name);
	}
}

module_init(firmware_init);
module_exit(firmware_exit);

MODULE_AUTHOR("huyanwei <srclib@hotmail.com>");
MODULE_DESCRIPTION("Firmware Driver.");
MODULE_LICENSE("GPL");
