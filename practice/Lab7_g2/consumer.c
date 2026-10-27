#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

#define NAME "/myShareMemory_lab7p"
#define MAX_MSG 10
#define MSG_LEN 100

struct shm_data {
    int count;
    char msg[MAX_MSG][MSG_LEN];
};

int main() {
    int fd = shm_open(NAME, O_RDONLY, 0666);   // 不加 O_CREAT：producer 要先跑
    if (fd == -1) {
        perror("Failed to open the shared memory");
        return 1;
    }
    struct shm_data *data = mmap(NULL, sizeof(struct shm_data),
                                 PROT_READ, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        perror("Failed to read the map");
        return 1;
    }

    printf("Read %d message(s) from shared memory:\n", data->count);
    for (int i = 0; i < data->count; i++)
        printf("[%d] %s\n", i + 1, data->msg[i]);

    munmap(data, sizeof(struct shm_data));
    close(fd);
    if (shm_unlink(NAME) == -1) {               // 最後讀完的人負責刪除
        perror("Failed to unlink the shared memory");
        return 1;
    }
    return 0;
}
