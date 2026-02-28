#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

int main() {
    int fd = open("/dev/kt_interrupt", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    char *msg = "Hello, interrupt!";
    write(fd, msg, strlen(msg) + 1);
    printf("Written: %s\n", msg);

    printf("Now reading (will block for ~5 seconds)...\n");
    char buf[256];
    int n = read(fd, buf, sizeof(buf));
    if (n > 0) {
        printf("Read: %s\n", buf);
    }

    close(fd);
    return 0;
}
