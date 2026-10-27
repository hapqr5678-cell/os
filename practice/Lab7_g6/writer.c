#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define NAME "/persistShm_lab7p"
#define SMSIZE 1024

int main() {
    int fd = shm_open(NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) { perror("shm_open"); return 1; }
    if (ftruncate(fd, SMSIZE) == -1) { perror("ftruncate"); return 1; }
    char *ptr = mmap(NULL, SMSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) { perror("mmap"); return 1; }

    strcpy(ptr, "original data");
    printf("writer: wrote \"%s\" and exit (no shm_unlink)\n", ptr);

    munmap(ptr, SMSIZE);
    close(fd);
    return 0;      // process 結束了，但 shared memory 物件還在（kernel persistence）
}
