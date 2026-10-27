// Q5-a：用 write() 一次寫 1 byte，共寫 1MB，量時間
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#define TOTAL (1024 * 1024)

int main() {
    struct timespec t1, t2;
    char c = 'A';
    int fd = open("out_write.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd == -1) { perror("open"); return 1; }

    clock_gettime(CLOCK_MONOTONIC, &t1);
    for (int i = 0; i < TOTAL; i++)
        write(fd, &c, 1);               // 每次都是一個 system call
    clock_gettime(CLOCK_MONOTONIC, &t2);
    close(fd);

    printf("%f\n", (t2.tv_sec - t1.tv_sec) + (t2.tv_nsec - t1.tv_nsec) / 1e9);
    return 0;
}
