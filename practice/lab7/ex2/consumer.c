#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>     // close()

#define NAME "/myShareMemory"
#define SMSIZE 1024

int main() {
    /* 開啟已存在的 shared memory（唯讀） */
    int fd = shm_open(NAME, O_RDONLY, 0666);
    if (fd == -1) {
        perror("Failed to open the shared memory");
        return 1;
    }

    /* 映射（唯讀、共享） */
    char *ptr = mmap(NULL, SMSIZE, PROT_READ, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("Failed to read the map");
        return 1;
    }

    /* 印出 shared memory 的內容 */
    printf("Read from shared memory: %s\n", ptr);

    /* 清理：解除映射、關閉 fd、刪除 shared memory object */
    if (munmap(ptr, SMSIZE) == -1) {
        perror("Failed to unmap");
        return 1;
    }
    if (close(fd) == -1) {
        perror("Failed to close the shared memory");
        return 1;
    }
    if (shm_unlink(NAME) == -1) {
        perror("Failed to unlink the shared memory");
        return 1;
    }
    return 0;
}
