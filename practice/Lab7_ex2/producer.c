#include <fcntl.h>      // O_* constants
#include <sys/stat.h>   // mode constants
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>     // ftruncate() & close()

#define NAME "/myShareMemory"
#define SMSIZE 1024

int main() {
    /* 建立（或開啟）shared memory object */
    int fd = shm_open(NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("Failed to open or create the shared memory");
        return 1;
    }

    /* 設定大小 */
    if (ftruncate(fd, SMSIZE) == -1) {
        perror("Failed to resize");
        return 1;
    }

    /* 映射到自己的虛擬位址空間（可讀可寫、共享） */
    char *ptr = mmap(NULL, SMSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("Failed to create the map");
        return 1;
    }

    /* 讓使用者輸入訊息，直接寫進 shared memory */
    printf("Enter the message: ");
    fflush(stdout);
    if (fgets(ptr, SMSIZE, stdin) == NULL)
        ptr[0] = '\0';
    ptr[strcspn(ptr, "\n")] = '\0';   // 去掉換行

    /* 解除映射、關閉 fd（不 unlink，資料要留給 consumer） */
    if (munmap(ptr, SMSIZE) == -1) {
        perror("Failed to unmap");
        return 1;
    }
    if (close(fd) == -1) {
        perror("Failed to close the shared memory");
        return 1;
    }
    return 0;
}
