#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME "/myMsgQueue"
#define MAX_MSGSIZE 1000

int main() {
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MSGSIZE;
    attr.mq_curmsgs = 0;

    /* 建立（或開啟）message queue，只讀 */
    mqd_t mqd = mq_open(NAME, O_RDONLY | O_CREAT, 0644, &attr);
    if (mqd == (mqd_t)-1) {
        perror("Failed to open or create the message queue");
        return 1;
    }

    /* buffer 大小必須 >= queue 的 mq_msgsize，所以先查詢實際值 */
    if (mq_getattr(mqd, &attr) == -1) {
        perror("Failed to get attributes");
        return 1;
    }
    char *buffer = malloc(attr.mq_msgsize);
    if (buffer == NULL) {
        perror("malloc");
        return 1;
    }

    while (1) {
        ssize_t msg_size = mq_receive(mqd, buffer, attr.mq_msgsize, NULL);
        if (msg_size == -1) {
            perror("Failed to read the message");
            free(buffer);
            return 1;
        }

        /* 收到 exit 就跳出迴圈 */
        if (strcmp(buffer, "exit") == 0) {
            printf("Exiting receive...\n");
            break;
        }
        printf("received: %s\n", buffer);
    }
    free(buffer);

    /* close the message queue: 0 -> success, -1 -> fail */
    if (mq_close(mqd) == -1) {
        perror("Failed to close the message queue");
        return 1;
    }

    /* unlink the message queue: 0 -> success, -1 -> fail */
    if (mq_unlink(NAME) == -1) {
        perror("Failed to unlink the message queue");
        return 1;
    }
    return 0;
}
