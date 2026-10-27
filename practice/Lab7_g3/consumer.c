#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

#define NAME "/numShm_lab7p"
#define MAX_N 100

struct shm_data {
    int n;
    int num[MAX_N];
};

int main() {
    int fd = shm_open(NAME, O_RDONLY, 0666);
    if (fd == -1) { perror("shm_open"); return 1; }
    struct shm_data *data = mmap(NULL, sizeof(struct shm_data),
                                 PROT_READ, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) { perror("mmap"); return 1; }

    if (data->n == 0) {
        printf("no data.\n");
    } else {
        int sum = 0, max = data->num[0], min = data->num[0];
        printf("Numbers:");
        for (int i = 0; i < data->n; i++) {
            printf(" %d", data->num[i]);
            sum += data->num[i];
            if (data->num[i] > max) max = data->num[i];
            if (data->num[i] < min) min = data->num[i];
        }
        printf("\nsum = %d, avg = %.2f, max = %d, min = %d\n",
               sum, (double)sum / data->n, max, min);
    }

    munmap(data, sizeof(struct shm_data));
    close(fd);
    shm_unlink(NAME);
    return 0;
}
