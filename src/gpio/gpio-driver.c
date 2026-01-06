#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

#include <linux/proc_fs.h>
#include <linux/slab.h>

#include <asm/io.h>

#define GPIO_BASE_ADDRESS					0x7E200000

static int __init gpio_driver_init(void) {
	printk("GPIO Driver Entry!\r\n");
	
	return 0;
}

static void __exit gpio_driver_exit(void) {
	printk("GPIO Driver clear!\r\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Iago Errera");
MODULE_DESCRIPTION("Simple GPIO driver for Raspberry Pi 4");
MODULE_VERSION("1.0");
