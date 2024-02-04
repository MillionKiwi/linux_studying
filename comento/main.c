#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/hashtable.h>
#define COMENTO_DEVICE_NAME	"ringbuffer"
#define COMENTO_CLASS_NAME	"comento"
#define QUEUE_SIZE	4096

static int comento_device_major;
static struct class *comento_class;
static struct device *comento_device;

static int front = 0, rear = 0;
char queue[QUEUE_SIZE] = {0, };
static DEFINE_RWLOCK(comento_device_rwlock);

static bool enqueue(char data){
	if (front == ((rear + 1) % QUEUE_SIZE)){
		return false;
	}
	queue[rear++] = data;
	return true;
}

static bool dequeue(char *data){
	if (front == rear){
		return false;
	}
	*data = queue[front--];
	return true;
}

static ssize_t comento_device_read(struct file *fp, char __user *buf, size_t len, loff_t *ppos){
	int i, read_bytes =0;
	char *kernel_buf = kmalloc(len, GFP_KERNEL);
	read_lock(&comento_device_rwlock);
	for (i=0; i<len; i++){
		if (!dequeue(kernel_buf + i)){	break;}
		read_bytes++;
	}
	read_unlock(&comento_device_rwlock);
	copy_to_user(buf, kernel_buf,len);
	kfree(kenel_buf);

	return read_bytes;
}

static ssize comento_device_write(struct file *fp, const char __user *buf, size_t len, loff_t *ppos){
	int i, written_bytes = 0;
	char *kernel_buf = kamlloc(len,GFP_KERNEL);
	copy_from_user(kernel_buf, buf, len);
	write_lock(&comento_device_rwlock);
	for (i=0; i<len; i++){
		if (!enqueue(buf[i])){	break;}
		written_bytes++;
	}
	write_unlock(&comento_device_rwlock);
	return written_bytes;
}

static struct file_operations comento_device_fops = {
	.read = comento_device_read,
	.write = comento_device_write.
};

static int __init comento_module_init(void){
	int ret = 0;
	comento_device_major = register_chrdev(0, COMENTO_DEVICE_NAME, &comento_device_fops);
	if (comento_device_major < 0){
		printk(KERN_ERR "%s: Failed to get major number", COMENTO_DEVICE_NAME);
		ret = comento_device_major;
		goto err_register_chrdev;
	}
	comento_class = class_create(THIS_MODULE, COMENTO_CLASS_NAME);
	if (IS_ERR(comento_class)){
		printk(KERN_ERR "%s: Failed to create class", COMENTO_DEVICE_NAME);
		ret = PTR_ERR(comento_class);
		goto err_class;
	}

	comento_device = device_create(comento_class, NULL, MKDEV(comento_device_major, 0), NULL,COMENTO_DEVICE_NAME);
	if (IS_ERR(comento_device)){
		ret = PTR_ERR(comento_device);
		goto err_device;
	}

	return ret;

err_device:
	class_destory(comento_class);
err_class:
	unregister_chrdev(comento_device_major, COMENTO_DEVICE_NAME);
err_register_chrdev:
	return ret;
}

static void __exit comento_module_exit(void){
	device_destory(comento_class, MKDEV(comento_device_major, 0));
	class_destory(comento_class);
	unregister_chrdev(comento_device_major, COMENTO_DEVICE_NAME);
}

module_init(comento_module_init);
module_exit(comento_module_exit);

MODULE_AUTHOR("LeeHyunHo");
MODULE_DESCRIPTION("Ringbuffer device driver for assignment");
MODULE_LICENSE("CPL v2");
