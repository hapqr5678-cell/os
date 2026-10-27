#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

#define NAME "/persistShm_lab7p"
#define SMSIZE 1024

int main() {
    int fd = shm_open(NAME, O_RDONLY, 0666);
    if (fd == -1) { perror("shm_open"); return 1; }
    char *ptr = mmap(NULL, SMSIZE, PROT_READ, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) { perror("mmap"); return 1; }
    printf("cleaner: final content = %s\n", ptr);
    munmap(ptr, SMSIZE);
    close(fd);
    if (shm_unlink(NAME) == -1) { perror("shm_unlink"); return 1; }
    printf("cleaner: shm_unlink done\n");
    return 0;
}
