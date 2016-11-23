#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mman.h>
#include <linux/random.h>
#include <linux/init.h>
#include <linux/raw.h>
#include <linux/tty.h>
#include <linux/capability.h>
#include <linux/ptrace.h>
#include <linux/device.h>
#include <linux/highmem.h>
#include <linux/crash_dump.h>
#include <linux/backing-dev.h>
#include <linux/bootmem.h>
#include <linux/splice.h>
#include <linux/pfn.h>
#include <linux/export.h>
#include <linux/io.h>
#include <linux/aio.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <asm/uaccess.h>

#define DEVICE_NAME     "slic_gpio_dev"  //¿¿¿¿¿¿¿¿¿cat /proc/devices¿¿¿¿¿¿¿¿¿¿ 
#define GPIO_MAJOR       0       //¿¿¿¿
#define delays	50
#define LA_GAO 1 
#define LA_DI 0  // fuwei
#define WATCHDOG_ENABLE 2
#define WATCHDOG_LOW 3
#define WATCHDOG_HIGHT 4

volatile unsigned long *GPIO_CTRL_1;
volatile unsigned long *GPIO_DATA_1; 
volatile unsigned long *GPIO1MODE;
volatile unsigned long *GPIO2MODE;
volatile unsigned long *SYSCFG0;

static struct class *gpio_rst_drv_class;

static int gpio_rst_drv_open(struct inode *inode, struct file *file)
{
	printk("%s:Hello gpio_rst\n", __FUNCTION__);	// printk
	return 0;
}

static long gpio_rst__drv_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	switch(cmd)
	{		
		case LA_DI:
			*GPIO_DATA_1 &=~(1<<10);
			//printk("%s:LA DI\n", __FUNCTION__);
			break;
		case LA_GAO:
			*GPIO_DATA_1 |=(1<<10);
			//printk("%s: LA GAO\n", __FUNCTION__);
			break;
		default:
			break;
	}
	return 0;
}

static struct file_operations gpio_rst_drv_fops = {
	.owner  	= THIS_MODULE,   
	.open   	= gpio_rst_drv_open,
	.unlocked_ioctl = gpio_rst__drv_ioctl,     
};

int major;


static int __init gpio_rst_drv_init(void)
{
	major = register_chrdev(GPIO_MAJOR, DEVICE_NAME, &gpio_rst_drv_fops);
	if (major < 0)
	{
		printk(DEVICE_NAME " can't register major number\n");
		return major;
	}
	
	gpio_rst_drv_class = class_create(THIS_MODULE, "slic_gpio");
	device_create(gpio_rst_drv_class, NULL, MKDEV(major, 0), NULL, "slic_gpio");		// /dev/slic_gpio
	printk("%s:Hello slic_gpio\n", __FUNCTION__);	// printk¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿printf¿¿
	SYSCFG0=(volatile unsigned long *)ioremap(0x10000010,4);
	//GPIO1MODE = (volatile unsigned long *)ioremap(0x10000060,4);
	GPIO2MODE=(volatile unsigned long *)ioremap(0x10000064,4);
	GPIO_CTRL_1=(volatile unsigned long *)ioremap(0x10000604,4);
	GPIO_DATA_1=(volatile unsigned long *)ioremap(0x10000624,4);
	*SYSCFG0 &= ~(1<<8);
	//*GPIO1MODE |=(1<<18);// ¿´ÃÅ¹·Î¹¹·Òý½Å
	*GPIO2MODE &=~(0x3<<4); 
	*GPIO2MODE|=(1<<4);
	//*GPIO2MODE &=~(0x3<<8); // ¿´ÃÅ¹·µÍÓÐÐ§Òý½Å
	//*GPIO2MODE |=(1<<8);
	*GPIO_CTRL_1|=(1<<10);// set output
		
	return 0;
}

static void __exit gpio_rst_drv_exit(void)
{
	unregister_chrdev(major, "gpio_rst");		// ¿¿¿¿¿¿register_chrdev¿¿¿¿¿¿
	device_destroy(gpio_rst_drv_class, MKDEV(major, 0));	// ¿¿¿¿¿¿device_create¿¿¿¿¿¿
	class_destroy(gpio_rst_drv_class);	// ¿¿¿¿¿¿class_create¿¿¿¿¿¿
	iounmap(GPIO2MODE);
	iounmap(GPIO_CTRL_1);
	iounmap(GPIO_CTRL_1);
	iounmap(GPIO_DATA_1);
	printk("%s:Hello gpio_rst\n", __FUNCTION__);	// printk¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿printf¿¿
}

module_init(gpio_rst_drv_init);
module_exit(gpio_rst_drv_exit);

MODULE_AUTHOR("http://www.gpio_rst.com");
MODULE_VERSION("0.1.0");
MODULE_DESCRIPTION("RT5350 FIRST Driver");
MODULE_LICENSE("GPL");

