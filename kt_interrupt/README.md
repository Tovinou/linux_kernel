# Character Device Driver with Interrupt Simulation

This project implements a Linux kernel module that creates a character device `/dev/kt_interrupt`. It demonstrates core kernel concepts: file operations, mutex locking, wait queues, and simulated interrupts using a kernel timer.

---

## Features

- Registers a character device with a dynamic major number.  
- Implements `open`, `read`, `write`, and `release` operations.  
- Uses a mutex to protect shared data (buffer).  
- Employs a wait queue to block reading processes until data is available.  
- Simulates an interrupt with a kernel timer: after a write, the timer expires after 5 seconds, marking data as ready and waking up any waiting readers.

---

## Project Structure

.
├── include/
│   └── kt_device.h          # Header with device structure and function prototypes
├── src/
│   ├── Makefile             # Build file for the kernel module
│   ├── main.c                # Module init/exit and file_operations definition
│   ├── kt_fops.c             # Implementation of open, read, write, release
│   ├── kt_timer.c            # Timer callback function
│   └── ... (build artifacts)
├── tests/
│   ├── test_1.c              # Test program (writes then reads, blocking)
│   ├── test.c                # Additional test (if any)
│   └── test_1                # Compiled test binary
└── README.md                 # This file


---

## Requirements

- Raspberry Pi (or any Linux system) with kernel headers installed.  
- Kernel headers matching your running kernel:

```bash
sudo apt install raspberrypi-kernel-headers   # on Raspberry Pi OS

## Build tools:

## Building the Module

- Navigate to the project root:

cd ~/kt_interrupt

## Build the module:
make

- The compiled module kt_interrupt.ko will be placed in the src/ directory.

Loading the Module

Insert the module into the kernel:

sudo insmod src/kt_interrupt.ko

Check kernel messages:

dmesg | tail

You should see:

kt_interrupt: module loaded successfully

The device file /dev/kt_interrupt should now exist (created automatically via udev).

Testing the Driver

Compile the test program:

cd tests
gcc test_1.c -o test_1

Run the test (may need sudo):

sudo ./test_1

Expected output:

Written: Hello, interrupt!
Now reading (will block for ~5 seconds)...
Read: Hello, interrupt!

While the read blocks, you can check dmesg to see the timer expiry message after 5 seconds:

kt_interrupt: timer expired (simulated interrupt)
kt_interrupt: read 17 bytes
Unloading the Module

Remove the module when done:

sudo rmmod kt_interrupt

Verify cleanup message:

dmesg | tail
kt_interrupt: module unloaded
Cleaning Up

To remove compiled files:

make clean

This deletes .o, .ko, .mod.*, and other build artifacts from src/.

Notes

Timer delay is hardcoded to 5 seconds. Modify it in kt_fops.c (msecs_to_jiffies(5000)).

Buffer size is 256 bytes – larger writes will be truncated.

The module uses a simple mutex; for multiple readers/writers, additional logic is needed.

Troubleshooting

Build errors about class_create:
Newer kernels use the one-argument class_create(). Edit src/main.c:

// old
kt_class = class_create(THIS_MODULE, "kt_interrupt");
// new
kt_class = class_create("kt_interrupt");

Permission denied on /dev/kt_interrupt:

sudo chmod 666 /dev/kt_interrupt

Module not unloading: Ensure no process has the device open:

lsof /dev/kt_interrupt
fuser -v /dev/kt_interrupt
License
