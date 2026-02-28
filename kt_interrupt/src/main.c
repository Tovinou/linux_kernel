#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include "kt_device.h"
#include "../include/kt_device.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Komlan");
MODULE_DESCRIPTION("Character device with interrupt simulation");

static int major;
static struct class *kt_class = NULL;
static struct device *kt_device = NULL;
static struct cdev kt_cdev;
struct kt_interrupt_dev dev_data;

static struct file_operations kt_interrupt_fops = {
    .owner   = THIS_MODULE,
    .open    = kt_interrupt_open,
    .release = kt_interrupt_release,
    .read    = kt_interrupt_read,
    .write   = kt_interrupt_write,
};

static int __init kt_interrupt_init(void) {
    dev_t dev_num;
    int ret;

    pr_info("kt_interrupt: initializing module\n");

    mutex_init(&dev_data.lock);
    init_waitqueue_head(&dev_data.wq);
    dev_data.data_ready = 0;
    dev_data.data_size = 0;
    timer_setup(&dev_data.timer, timer_callback, 0);

    ret = alloc_chrdev_region(&dev_num, 0, 1, "kt_interrupt");
    if (ret < 0) return ret;
    major = MAJOR(dev_num);

    cdev_init(&kt_cdev, &kt_interrupt_fops);
    kt_cdev.owner = THIS_MODULE;
    ret = cdev_add(&kt_cdev, dev_num, 1);
    if (ret < 0) goto err_cdev;

    kt_class = class_create("kt_interrupt");
    if (IS_ERR(kt_class)) {
        ret = PTR_ERR(kt_class);
        goto err_class;
    }

    kt_device = device_create(kt_class, NULL, dev_num, NULL, "kt_interrupt");
    if (IS_ERR(kt_device)) {
        ret = PTR_ERR(kt_device);
        goto err_device;
    }

    pr_info("kt_interrupt: module loaded successfully\n");
    return 0;

err_device:
    class_destroy(kt_class);
err_class:
    cdev_del(&kt_cdev);
err_cdev:
    unregister_chrdev_region(dev_num, 1);
    return ret;
}

static void __exit kt_interrupt_exit(void) {
    dev_t dev_num = MKDEV(major, 0);

    del_timer_sync(&dev_data.timer);
    device_destroy(kt_class, dev_num);
    class_destroy(kt_class);
    cdev_del(&kt_cdev);
    unregister_chrdev_region(dev_num, 1);

    pr_info("kt_interrupt: module unloaded\n");
}

module_init(kt_interrupt_init);
module_exit(kt_interrupt_exit);
