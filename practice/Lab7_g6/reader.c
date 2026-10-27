#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define NAME "/persistShm_lab7p"
#define SMSIZE 1024

int main() {
    int fd = shm_open(NAME, O_RDWR, 0666);
    if (fd == -1) { perror("shm_open"); return 1; }

    /* (1) MAP_PRIVATE：改了只影響自己（copy-on-write），不會寫回 shared memory */
    char *priv = mmap(NULL, SMSIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (priv == MAP_FAILED) { perror("mmap private"); return 1; }
    printf("[PRIVATE] before: %s\n", priv);
    strcpy(priv, "modified by MAP_PRIVATE");
    printf("[PRIVATE] after : %s\n", priv);

    /* (2) MAP_SHARED：看到的是真正 shared memory 的內容 */
    char *shared = mmap(NULL, SMSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared == MAP_FAILED) { perror("mmap shared"); return 1; }
    printf("[SHARED ] content: %s\n", shared);

    strcpy(shared, "modified by MAP_SHARED");
    printf("[SHARED ] after : %s\n", shared);

    munmap(priv, SMSIZE);
    munmap(shared, SMSIZE);
    close(fd);
    return 0;
}
