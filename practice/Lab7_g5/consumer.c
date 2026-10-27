#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define SHM_NAME "/dataShm_lab7p"
#define NOTE_NAME "/noteQ_lab7p"
#define ACK_NAME  "/ackQ_lab7p"
#define SMSIZE 1024
#define MAX_MSGSIZE 64

int main() {
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MSGSIZE;
    attr.mq_curmsgs = 0;
    mqd_t note = mq_open(NOTE_NAME, O_RDONLY | O_CREAT, 0644, &attr);
    mqd_t ack  = mq_open(ACK_NAME,  O_WRONLY | O_CREAT, 0644, &attr);
    if (note == (mqd_t)-1 || ack == (mqd_t)-1) { perror("mq_open"); return 1; }

    printf("Waiting for producer...\n");
    fflush(stdout);
    char buf[MAX_MSGSIZE];
    /* 先等第一個通知：確定 producer 已經建立好 shared memory */
    if (mq_receive(note, buf, MAX_MSGSIZE, NULL) == -1) { perror("mq_receive"); return 1; }

    int fd = shm_open(SHM_NAME, O_RDONLY, 0666);
    if (fd == -1) { perror("shm_open"); return 1; }
    char *ptr = mmap(NULL, SMSIZE, PROT_READ, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) { perror("mmap"); return 1; }

    while (1) {
        if (strcmp(ptr, "exit") == 0) {
            printf("Producer finished.\n");
            break;
        }
        printf("Read from shared memory: %s\n", ptr);
        fflush(stdout);
        mq_send(ack, "done", 5, 0);                               // 告訴 producer 可以寫下一筆
        if (mq_receive(note, buf, MAX_MSGSIZE, NULL) == -1) {     // 等下一個通知
            perror("mq_receive"); return 1;
        }
    }

    munmap(ptr, SMSIZE);
    close(fd);
    mq_close(note);
    mq_close(ack);
    /* consumer 最後結束，負責清理全部 IPC 物件 */
    shm_unlink(SHM_NAME);
    mq_unlink(NOTE_NAME);
    mq_unlink(ACK_NAME);
    return 0;
}
