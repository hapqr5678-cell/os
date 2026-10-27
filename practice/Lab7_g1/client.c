#include <fcntl.h>      // O_* constants
#include <sys/stat.h>   // mode constants
#include <mqueue.h>
#include <stdio.h>
#include <string.h>

#define REQ_NAME "/calcReq_lab7p"   // client -> server
#define RES_NAME "/calcRes_lab7p"   // server -> client
#define MAX_MSGSIZE 128

int main() {
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MSGSIZE;
    attr.mq_curmsgs = 0;

    /* 兩邊都加 O_CREAT，誰先執行都可以 */
    mqd_t req = mq_open(REQ_NAME, O_WRONLY | O_CREAT, 0644, &attr);
    mqd_t res = mq_open(RES_NAME, O_RDONLY | O_CREAT, 0644, &attr);
    if (req == (mqd_t)-1 || res == (mqd_t)-1) {
        perror("Failed to open or create the message queue");
        return 1;
    }

    printf("Enter expression (e.g. 3 + 5), type exit to quit:\n");
    while (1) {
        char buffer[MAX_MSGSIZE];
        memset(buffer, '\0', sizeof(buffer));
        printf("> ");
        fflush(stdout);
        if (fgets(buffer, MAX_MSGSIZE, stdin) == NULL)
            strcpy(buffer, "exit");
        buffer[strcspn(buffer, "\n")] = '\0';

        if (mq_send(req, buffer, strlen(buffer) + 1, 0) == -1) {
            perror("Failed to send a message");
            return 1;
        }
        if (strcmp(buffer, "exit") == 0)
            break;

        /* 等 server 回傳結果（buffer 大小 >= mq_msgsize） */
        char result[MAX_MSGSIZE];
        if (mq_receive(res, result, MAX_MSGSIZE, NULL) == -1) {
            perror("Failed to read the message");
            return 1;
        }
        printf("result: %s\n", result);
    }

    mq_close(req);
    mq_close(res);
    return 0;
}
