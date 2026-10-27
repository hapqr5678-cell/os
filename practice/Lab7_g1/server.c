#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <stdio.h>
#include <string.h>

#define REQ_NAME "/calcReq_lab7p"
#define RES_NAME "/calcRes_lab7p"
#define MAX_MSGSIZE 128

int main() {
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MSGSIZE;
    attr.mq_curmsgs = 0;

    mqd_t req = mq_open(REQ_NAME, O_RDONLY | O_CREAT, 0644, &attr);
    mqd_t res = mq_open(RES_NAME, O_WRONLY | O_CREAT, 0644, &attr);
    if (req == (mqd_t)-1 || res == (mqd_t)-1) {
        perror("Failed to open or create the message queue");
        return 1;
    }

    printf("Server is waiting...\n");
    fflush(stdout);
    while (1) {
        char buffer[MAX_MSGSIZE];
        if (mq_receive(req, buffer, MAX_MSGSIZE, NULL) == -1) {
            perror("Failed to read the message");
            return 1;
        }
        if (strcmp(buffer, "exit") == 0) {
            printf("Server exit.\n");
            break;
        }

        int a, b;
        char op;
        char result[MAX_MSGSIZE];
        if (sscanf(buffer, "%d %c %d", &a, &op, &b) != 3)
            sprintf(result, "invalid input");
        else if (op == '+') sprintf(result, "%d", a + b);
        else if (op == '-') sprintf(result, "%d", a - b);
        else if (op == '*') sprintf(result, "%d", a * b);
        else if (op == '/' && b != 0) sprintf(result, "%d", a / b);
        else sprintf(result, "invalid input");

        printf("received: %s -> %s\n", buffer, result);
        fflush(stdout);
        if (mq_send(res, result, strlen(result) + 1, 0) == -1) {
            perror("Failed to send a message");
            return 1;
        }
    }

    mq_close(req);
    mq_close(res);
    /* server 最後結束，負責刪除兩個 queue */
    if (mq_unlink(REQ_NAME) == -1 || mq_unlink(RES_NAME) == -1) {
        perror("Failed to unlink the message queue");
        return 1;
    }
    return 0;
}
