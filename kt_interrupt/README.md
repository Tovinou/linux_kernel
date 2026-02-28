Character Device Driver with Interrupt Simulation
A Linux kernel module project demonstrating the implementation of a character device (/dev/kt_interrupt) using core kernel synchronization and scheduling primitives.

Overview
This project simulates hardware behavior using a kernel timer. It showcases how to manage process blocking and unblocking in kernel space, ensuring data integrity across multiple operations.

Key Features
Dynamic Registration: Automatically assigns a major number and creates a device node.

Thread Safety: Uses a mutex to prevent race conditions during buffer access.

Event-Driven Blocking: Implements a wait_queue to put reading processes to sleep until data is ready.

Interrupt Simulation: Employs a kernel timer to simulate an asynchronous interrupt 5 seconds after a write operation.

Project Structure
Plaintext
.
├── include/
│   └── kt_device.h      # Device structure and function prototypes
├── src/
│   ├── Makefile         # Kernel build system file
│   ├── main.c           # Module init/exit and file_operations
│   ├── kt_fops.c        # Implementation of open, read, write, release
│   └── kt_timer.c       # Timer callback (The "Interrupt" simulator)
├── tests/
│   ├── test_1.c         # Main test program (Blocking Read/Write)
│   └── test.c           # Utility tests
└── README.md            # Project documentation
Getting Started
Requirements
Linux System: Tested on Raspberry Pi OS / Ubuntu.

Kernel Headers: Ensure headers match your running kernel version.

Bash
sudo apt update
sudo apt install raspberrypi-kernel-headers  # For Raspberry Pi
# OR
sudo apt install linux-headers-$(uname -r)   # For Generic Linux
Building the Module
Navigate to the project root:

Bash
cd ~/kt_interrupt
Compile the module using the provided Makefile:

Bash
make
The compiled driver kt_interrupt.ko will be generated in the src/ directory.

Usage
1. Loading the Module
Insert the module into the kernel:

Bash
sudo insmod src/kt_interrupt.ko
Verify the installation by checking the kernel log:

Bash
dmesg | tail
# Expected: kt_interrupt: module loaded successfully
2. Testing the Driver
Compile and run the user-space test application:

Bash
cd tests
gcc test_1.c -o test_1
sudo ./test_1
Expected Behavior:

The program writes "Hello, interrupt!" to the device.

The program attempts to read and immediately blocks.

After 5 seconds, the kernel timer expires (simulating an interrupt).

The timer wakes up the reading process, and the message is displayed.

3. Unloading the Module
Bash
sudo rmmod kt_interrupt
Technical Details
Logic Flow
Write: Data is copied from user-space to a 256-byte internal buffer. A kernel timer is triggered.

Read: If the "data ready" flag is false, the process is added to a wait_queue and put to sleep (Interruptible).

Timer Expiry: After 5000ms, the timer callback sets the flag to true and calls wake_up_interruptible().

Wake Up: The reading process resumes, copies data to user-space, and clears the buffer.

Troubleshooting
Class Creation Errors: If using a kernel version > 6.4, the class_create macro changed. Ensure your main.c matches:

C
// Modern kernels (1 argument)
kt_class = class_create("kt_interrupt");
Permissions: If you cannot access the device without sudo, run:

Bash
sudo chmod 666 /dev/kt_interrupt
License
This project is licensed under the GPLv2.
