#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

#include <linux/proc_fs.h>
#include <linux/slab.h>

#include <linux/uaccess.h>

#include <asm/io.h>

#define GPIO_BASE_ADDRESS    	0xFE200000
#define GPIO_BLOCK_SIZE      	4096

#define GPIO_GPFSEL_OFFSET	0x00
#define GPIO_GPSET_OFFSET	0x1C
#define GPIO_GPCLR_OFFSET	0x28
#define GPIO_GPLEV_OFFSET	0x34
#define GPIO_GPEDS_OFFSET	0x40
#define GPIO_GPREN_OFFSET	0x4C
#define GPIO_GPFEN_OFFSET 	0x58
#define GPIO_GPHEN_OFFSET	0x64
#define GPIO_GPLEN_OFFSET	0x70
#define GPIO_GPAREN_OFFSET	0x7C
#define GPIO_GPAFEN_OFFSET	0x88
#define GPIO_PUP_PDN_CNTRL	0xE4

#define REG_SIZE		4
#define GET_REG(BASE, IDX) (gpio_base + BASE + (IDX * REG_SIZE))

#define BUFFER_SIZE		1024
#define ACTION_BUFFER_SIZE	10

ssize_t gpio_read(struct file* file, char __user* user, size_t size, loff_t* off);
ssize_t gpio_write(struct file* file, const char __user* user, size_t size, loff_t* off);
void write_pin(unsigned int pin, unsigned int value);
void set_pin_mode(unsigned int pin, unsigned int value);

static struct proc_dir_entry* gpio_proc = NULL; 
void __iomem *gpio_base;

static const struct proc_ops gpio_fops = {
	.proc_read  = gpio_read,
	.proc_write = gpio_write
};

static char buffer[BUFFER_SIZE];
static char action[ACTION_BUFFER_SIZE];

static int __init gpio_driver_init(void) {
	printk("[INFO] Loading GPIO Driver\r\n");
	
	if (!(gpio_base = ioremap(GPIO_BASE_ADDRESS, GPIO_BLOCK_SIZE))) {
		printk(KERN_ERR "[ERROR] Memory map fail\r\n");
		return -ENOMEM;
	}
	
	if ((gpio_proc = proc_create("gpio", 0666, NULL, &gpio_fops)) == NULL) {
		printk(KERN_ERR "[ERROR] Fail on create gpio proc\r\n!");
		iounmap(gpio_base);
		return -1;
	}

	printk("[INFO] GPIO Driver loaded\r\n");

	return 0;
}

static void __exit gpio_driver_exit(void) {
	printk("[INFO] Unloading GPIO Driver\r\n");

	if (gpio_proc) proc_remove(gpio_proc);
	if (gpio_base) iounmap(gpio_base);
}


ssize_t gpio_read(struct file* file, char __user* user, size_t size, loff_t* off) {
	return 0;
}

ssize_t gpio_write(struct file* file, const char __user* user, size_t size, loff_t* off) {
	int n;
	memset(buffer, 0, sizeof(buffer));
	
	if (size > BUFFER_SIZE) size = BUFFER_SIZE;
	if ((n = copy_from_user(buffer, user, size)) != 0) {
		printk("[ERROR] Fail on copy from user buffer - %d bytes can't be copied\r\n", n);
		return -EFAULT;
	}

	printk("[INFO] Data received: %s\r\n", buffer);	

	unsigned int pin = UINT_MAX, value = UINT_MAX;
	if (sscanf(buffer, "%9[^,],%d,%d", action, &pin, &value) != 3) {
		printk("[ERROR] Invalid data format. Expected \"action,pin,value\"\r\n");
		return size;
	}
	
	if (pin > 26) {
		printk("[ERROR] Invalid pin\r\n");
		return size;
	}

	if (value != 0 && value != 1) {
		printk("[ERROR] Invalid value\r\n");
		return size;
	}

	
	if (strcmp(action, "write") == 0) write_pin(pin,value);
	else if (strcmp(action, "mode") == 0) set_pin_mode(pin,value);
	else printk("[ERRO] Invalid action\r\n");

	return size;
}

void set_pin_mode(unsigned int pin, unsigned int value) {
	if (value > 7) return;
	
	unsigned int reg_idx = pin / 10; 
	unsigned int bit_off = 3 * (pin % 10); 
		
	unsigned int val = ioread32(GET_REG(GPIO_GPFSEL_OFFSET, reg_idx));
	val &= ~(0b111<<bit_off);
	val |= (value<<bit_off);
	iowrite32(val, GET_REG(GPIO_GPFSEL_OFFSET, reg_idx));

	printk("Setting pin %d to mode %d\r\n", pin, value);
}

int get_pin_input(unsigned int pin) {
	unsigned int reg_idx = pin / 10; 
	unsigned int bit_off = 3 * (pin % 10); 
		
	unsigned int val = ioread32(GET_REG(GPIO_GPFSEL_OFFSET, reg_idx));
	unsigned int mode = val>>bit_off & (0b111);

	if (mode != 0) {
		printk("Pin %d setted to mode %d\r\n", pin, mode)
		return -1;
	}

	reg_idx = pin / 32;
	bit_off = pin % 32;

	val = ioread32(GET_REG(GPIO_GPLEV_OFFSET, reg_idx));
	return (val>>bit_off) & 0x1;
}	

void write_pin(unsigned int pin, unsigned int value) {
	unsigned int base = (value ? GPIO_GPSET_OFFSET : GPIO_GPCLR_OFFSET);
	unsigned int reg_idx = pin / 32; 
	unsigned int bit_off = pin % 32; 
	
	unsigned int val = (1U<<bit_off);
	iowrite32(val, GET_REG(base, reg_idx));
	
	printk("Changing pin %d to %d\r\n", pin, value);
}

module_init(gpio_driver_init);
module_exit(gpio_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Iago Errera");
MODULE_DESCRIPTION("Simple GPIO driver for Raspberry Pi 4");
MODULE_VERSION("1.0");
