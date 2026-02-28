#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main() {
    int fd = open("/dev/kt_interrupt", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    printf("Opened device\n");

    write(fd, "test", 4);

    char buf[100] = {0};
    int n = read(fd, buf, sizeof(buf));

    if (n < 0) {
        perror("read");
    } else {
        printf("Read %d bytes: %s\n", n, buf);
    }

    close(fd);
    return 0;
}
