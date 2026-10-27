#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define SHM_NAME "/dataShm_lab7p"
#define NOTE_NAME "/noteQ_lab7p"    // producer -> consumer：「資料好了」
#define ACK_NAME  "/ackQ_lab7p"     // consumer -> producer：「我讀完了」
#define SMSIZE 1024
#define MAX_MSGSIZE 64

int main() {
    /* 1. shared memory：放真正的資料 */
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) { perror("shm_open"); return 1; }
    if (ftruncate(fd, SMSIZE) == -1) { perror("ftruncate"); return 1; }
    char *ptr = mmap(NULL, SMSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) { perror("mmap"); return 1; }

    /* 2. message queue：只用來「通知」 */
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MSGSIZE;
    attr.mq_curmsgs = 0;
    mqd_t note = mq_open(NOTE_NAME, O_WRONLY | O_CREAT, 0644, &attr);
    mqd_t ack  = mq_open(ACK_NAME,  O_RDONLY | O_CREAT, 0644, &attr);
    if (note == (mqd_t)-1 || ack == (mqd_t)-1) { perror("mq_open"); return 1; }

    while (1) {
        printf("Enter the message: ");
        fflush(stdout);
        if (fgets(ptr, SMSIZE, stdin) == NULL)      // 直接寫進 shared memory
            strcpy(ptr, "exit");
        ptr[strcspn(ptr, "\n")] = '\0';

        if (mq_send(note, "ready", 6, 0) == -1) { perror("mq_send"); return 1; }
        if (strcmp(ptr, "exit") == 0)
            break;

        char buf[MAX_MSGSIZE];
        if (mq_receive(ack, buf, MAX_MSGSIZE, NULL) == -1) {   // 等 consumer 讀完才覆寫
            perror("mq_receive"); return 1;
        }
    }

    munmap(ptr, SMSIZE);
    close(fd);
    mq_close(note);
    mq_close(ack);
    return 0;
}
