#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>     // ftruncate() & close()

#define NAME "/myShareMemory_lab7p"
#define MAX_MSG 10
#define MSG_LEN 100

/* shared memory 裡放的資料格式：兩邊必須「一模一樣」 */
struct shm_data {
    int count;                   // 已寫入幾筆
    char msg[MAX_MSG][MSG_LEN];  // 每筆訊息
};

int main() {
    int fd = shm_open(NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("Failed to open or create the shared memory");
        return 1;
    }
    if (ftruncate(fd, sizeof(struct shm_data)) == -1) {
        perror("Failed to resize");
        return 1;
    }
    struct shm_data *data = mmap(NULL, sizeof(struct shm_data),
                                 PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        perror("Failed to create the map");
        return 1;
    }

    data->count = 0;
    printf("Enter messages (type exit to finish):\n");
    while (data->count < MAX_MSG) {
        char buffer[MSG_LEN];
        printf("> ");
        fflush(stdout);
        if (fgets(buffer, MSG_LEN, stdin) == NULL)
            break;
        buffer[strcspn(buffer, "\n")] = '\0';
        if (strcmp(buffer, "exit") == 0)
            break;
        strcpy(data->msg[data->count], buffer);  // 寫進 shared memory
        data->count++;
    }
    printf("%d message(s) written to shared memory.\n", data->count);

    munmap(data, sizeof(struct shm_data));
    close(fd);        // 不 unlink：資料要留給 consumer
    return 0;
}
