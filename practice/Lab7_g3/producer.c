#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#define NAME "/numShm_lab7p"
#define MAX_N 100

struct shm_data {
    int n;
    int num[MAX_N];
};

int main() {
    int fd = shm_open(NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) { perror("shm_open"); return 1; }
    if (ftruncate(fd, sizeof(struct shm_data)) == -1) { perror("ftruncate"); return 1; }
    struct shm_data *data = mmap(NULL, sizeof(struct shm_data),
                                 PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) { perror("mmap"); return 1; }

    data->n = 0;
    printf("Enter integers (type exit to finish):\n");
    char buffer[64];
    while (data->n < MAX_N) {
        printf("> ");
        fflush(stdout);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
            break;
        buffer[strcspn(buffer, "\n")] = '\0';
        if (strcmp(buffer, "exit") == 0)
            break;
        data->num[data->n++] = atoi(buffer);   // 存「整數」，不是字串
    }
    printf("%d number(s) written.\n", data->n);

    munmap(data, sizeof(struct shm_data));
    close(fd);
    return 0;
}
