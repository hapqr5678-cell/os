#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define NAME "/prioQueue_lab7p"

int main() {
    /* O_NONBLOCK：queue 空了 mq_receive 不會卡住，而是回傳 -1 (errno = EAGAIN) */
    mqd_t mqd = mq_open(NAME, O_RDONLY | O_NONBLOCK);
    if (mqd == (mqd_t)-1) {
        perror("Failed to open the message queue");
        return 1;
    }

    struct mq_attr attr;
    if (mq_getattr(mqd, &attr) == -1) {
        perror("mq_getattr");
        return 1;
    }
    printf("maxmsg=%ld, msgsize=%ld, curmsgs=%ld\n",
           attr.mq_maxmsg, attr.mq_msgsize, attr.mq_curmsgs);

    char *buffer = malloc(attr.mq_msgsize);   // buffer 必須 >= mq_msgsize
    unsigned int prio;
    while (1) {
        ssize_t n = mq_receive(mqd, buffer, attr.mq_msgsize, &prio);
        if (n == -1) {
            if (errno == EAGAIN) {            // queue 已經空了
                printf("queue is empty.\n");
                break;
            }
            perror("Failed to read the message");
            return 1;
        }
        printf("received (prio %u): %s\n", prio, buffer);
    }
    free(buffer);

    mq_close(mqd);
    mq_unlink(NAME);
    return 0;
}
