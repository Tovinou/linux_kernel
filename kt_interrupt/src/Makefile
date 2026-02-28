# Name of the module (the .ko file will be kt_interrupt.ko)
obj-m := kt_interrupt.o

# List of object files that make up the module
kt_interrupt-objs := main.o kt_fops.o kt_timer.o

# Tell compiler where to find your headers
ccflags-y := -I$(src)/../include
