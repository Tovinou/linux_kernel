#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/jiffies.h>
#include "kt_device.h"

extern struct kt_interrupt_dev dev_data;

int kt_interrupt_open(struct inode *inode, struct file *filp) {
    pr_info("kt_interrupt: device opened\n");
    return 0;
}

int kt_interrupt_release(struct inode *inode, struct file *filp) {
    pr_info("kt_interrupt: device closed\n");
    return 0;
}

ssize_t kt_interrupt_read(struct file *filp, char __user *buf,
                          size_t count, loff_t *f_pos) {
    ssize_t ret;
    size_t len;

    if (mutex_lock_interruptible(&dev_data.lock))
        return -ERESTARTSYS;

    while (!READ_ONCE(dev_data.data_ready)) {
        mutex_unlock(&dev_data.lock);
        if (wait_event_interruptible(dev_data.wq, READ_ONCE(dev_data.data_ready)))
            return -ERESTARTSYS;
        if (mutex_lock_interruptible(&dev_data.lock))
            return -ERESTARTSYS;
    }

    del_timer_sync(&dev_data.timer);

    len = min(count, dev_data.data_size);
    if (copy_to_user(buf, dev_data.buffer, len)) {
        mutex_unlock(&dev_data.lock);
        return -EFAULT;
    }

    dev_data.data_size = 0;
    WRITE_ONCE(dev_data.data_ready, 0);

    mutex_unlock(&dev_data.lock);

    pr_info("kt_interrupt: read %zu bytes\n", len);
    return len;
}

ssize_t kt_interrupt_write(struct file *filp, const char __user *buf,
                           size_t count, loff_t *f_pos) {
    size_t len = min(count, sizeof(dev_data.buffer));

    if (mutex_lock_interruptible(&dev_data.lock))
        return -ERESTARTSYS;

    del_timer_sync(&dev_data.timer);

    if (copy_from_user(dev_data.buffer, buf, len)) {
        mutex_unlock(&dev_data.lock);
        return -EFAULT;
    }

    dev_data.data_size = len;
    WRITE_ONCE(dev_data.data_ready, 0);

    mod_timer(&dev_data.timer, jiffies + msecs_to_jiffies(5000));

    mutex_unlock(&dev_data.lock);

    pr_info("kt_interrupt: data stored, timer armed\n");
    return len;
}
