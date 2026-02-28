#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Komlan");
MODULE_DESCRIPTION("Character device with interrupt simulation using timer");

/* ===============================
   Global device variables
   =============================== */

static int major;                     // Major number
static struct class *kt_class = NULL; // Device class
static struct device *kt_device = NULL;
static struct cdev kt_cdev;

/* ===============================
   Device structure
   =============================== */

struct kt_interrupt_dev {
    struct mutex lock;            // Protect shared data
    wait_queue_head_t wq;         // Wait queue for blocking read
    int data_ready;               // Flag: interrupt occurred?
    char buffer[256];             // Data to send to user
    size_t data_size;             // Size of valid data
    struct timer_list timer;      // Kernel timer (simulated interrupt)
};

static struct kt_interrupt_dev dev_data;

/* ===============================
   Timer Callback (Interrupt Simulation)
   =============================== */

static void kt_timer_callback(struct timer_list *t)
{
    struct kt_interrupt_dev *dev =
        from_timer(dev, t, timer);

    pr_info("kt_interrupt: Timer fired (simulated interrupt)\n");

    mutex_lock(&dev->lock);

    dev->data_ready = 1;
    dev->data_size = snprintf(dev->buffer,
                              sizeof(dev->buffer),
                              "Interrupt occurred!\n");

    mutex_unlock(&dev->lock);

    // Wake up any process waiting in read()
    wake_up_interruptible(&dev->wq);
}

/* ===============================
   File Operations
   =============================== */

static int kt_interrupt_open(struct inode *inode, struct file *filp)
{
    pr_info("kt_interrupt: device opened\n");
    return 0;
}

static int kt_interrupt_release(struct inode *inode, struct file *filp)
{
    pr_info("kt_interrupt: device closed\n");
    return 0;
}

/* Blocking read */
static ssize_t kt_interrupt_read(struct file *filp,
                                 char __user *buf,
                                 size_t count,
                                 loff_t *f_pos)
{
    int ret;

    pr_info("kt_interrupt: read called, waiting for interrupt...\n");

    // Block until interrupt occurs
    wait_event_interruptible(dev_data.wq,
                             dev_data.data_ready != 0);

    mutex_lock(&dev_data.lock);

    if (*f_pos >= dev_data.data_size) {
        mutex_unlock(&dev_data.lock);
        return 0;  // EOF
    }

    if (count > dev_data.data_size)
        count = dev_data.data_size;

    ret = copy_to_user(buf, dev_data.buffer, count);
    if (ret) {
        mutex_unlock(&dev_data.lock);
        return -EFAULT;
    }

    dev_data.data_ready = 0;  // Reset flag
    *f_pos += count;

    mutex_unlock(&dev_data.lock);

    return count;
}

/* Write triggers the timer (simulates interrupt after 2 seconds) */
static ssize_t kt_interrupt_write(struct file *filp,
                                  const char __user *buf,
                                  size_t count,
                                  loff_t *f_pos)
{
    pr_info("kt_interrupt: write called, scheduling interrupt\n");

    // Schedule timer 2 seconds later
    mod_timer(&dev_data.timer,
              jiffies + msecs_to_jiffies(2000));

    return count;
}

/* File operations structure */
static struct file_operations kt_interrupt_fops = {
    .owner   = THIS_MODULE,
    .open    = kt_interrupt_open,
    .release = kt_interrupt_release,
    .read    = kt_interrupt_read,
    .write   = kt_interrupt_write,
};

/* ===============================
   Module Init
   =============================== */

static int __init kt_interrupt_init(void)
{
    dev_t dev_num;
    int ret;

    pr_info("kt_interrupt: Initializing module\n");

    // Initialize synchronization primitives
    mutex_init(&dev_data.lock);
    init_waitqueue_head(&dev_data.wq);
    dev_data.data_ready = 0;
    dev_data.data_size = 0;

    // Setup timer
    timer_setup(&dev_data.timer, kt_timer_callback, 0);

    // Allocate major number dynamically
    ret = alloc_chrdev_region(&dev_num, 0, 1, "kt_interrupt");
    if (ret < 0)
        return ret;

    major = MAJOR(dev_num);

    // Initialize and add cdev
    cdev_init(&kt_cdev, &kt_interrupt_fops);
    ret = cdev_add(&kt_cdev, dev_num, 1);
    if (ret < 0)
        goto unregister;

    // Create device class
    kt_class = class_create("kt_interrupt");
    if (IS_ERR(kt_class)) {
        ret = PTR_ERR(kt_class);
        goto del_cdev;
    }

    // Create /dev node
    kt_device = device_create(kt_class, NULL,
                              dev_num, NULL,
                              "kt_interrupt");
    if (IS_ERR(kt_device)) {
        ret = PTR_ERR(kt_device);
        goto destroy_class;
    }

    pr_info("kt_interrupt: Module loaded successfully\n");
    return 0;

destroy_class:
    class_destroy(kt_class);
del_cdev:
    cdev_del(&kt_cdev);
unregister:
    unregister_chrdev_region(dev_num, 1);
    return ret;
}

/* ===============================
   Module Exit
   =============================== */

static void __exit kt_interrupt_exit(void)
{
    dev_t dev_num = MKDEV(major, 0);

    pr_info("kt_interrupt: Unloading module\n");

    del_timer_sync(&dev_data.timer);

    device_destroy(kt_class, dev_num);
    class_destroy(kt_class);
    cdev_del(&kt_cdev);
    unregister_chrdev_region(dev_num, 1);

    pr_info("kt_interrupt: Module unloaded\n");
}

module_init(kt_interrupt_init);
module_exit(kt_interrupt_exit);