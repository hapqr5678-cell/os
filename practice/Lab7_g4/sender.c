#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <stdio.h>
#include <string.h>

#define NAME "/prioQueue_lab7p"
#define MAX_MSGSIZE 256

int main() {
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MSGSIZE;
    attr.mq_curmsgs = 0;

    mqd_t mqd = mq_open(NAME, O_WRONLY | O_CREAT, 0644, &attr);
    if (mqd == (mqd_t)-1) {
        perror("Failed to open or create the message queue");
        return 1;
    }

    printf("Input format: <priority> <message>, type exit to quit\n");
    while (1) {
        char line[MAX_MSGSIZE];
        printf("> ");
        fflush(stdout);
        if (fgets(line, MAX_MSGSIZE, stdin) == NULL)
            break;
        line[strcspn(line, "\n")] = '\0';
        if (strcmp(line, "exit") == 0)
            break;

        unsigned int prio;
        char msg[MAX_MSGSIZE];
        if (sscanf(line, "%u %[^\n]", &prio, msg) != 2) {
            printf("format error\n");
            continue;
        }
        /* 第 4 個參數就是 priority：數字越大越先被收 */
        if (mq_send(mqd, msg, strlen(msg) + 1, prio) == -1) {
            perror("Failed to send a message");
            return 1;
        }
    }
    mq_close(mqd);      // 只 close：訊息留在 queue 裡給 receiver
    return 0;
}
