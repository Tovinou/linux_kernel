#include <linux/kernel.h>
#include <linux/wait.h>
#include <linux/timer.h>
#include "kt_device.h"

extern struct kt_interrupt_dev dev_data;

void timer_callback(struct timer_list *t) {
    pr_info("kt_interrupt: timer expired (simulated interrupt)\n");
    WRITE_ONCE(dev_data.data_ready, 1);
    wake_up_interruptible(&dev_data.wq);
}
