#ifndef KT_DEVICE_H
#define KT_DEVICE_H

#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/timer.h>

struct kt_interrupt_dev {
    struct mutex lock;
    wait_queue_head_t wq;
    int data_ready;
    char buffer[256];
    size_t data_size;
    struct timer_list timer;
};

/* Function prototypes */
int kt_interrupt_open(struct inode *inode, struct file *filp);
int kt_interrupt_release(struct inode *inode, struct file *filp);
ssize_t kt_interrupt_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
ssize_t kt_interrupt_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);
void timer_callback(struct timer_list *t);

#endif /* KT_DEVICE_H */
