#include <fcntl.h>      // O_* constants
#include <sys/stat.h>   // mode constants
#include <mqueue.h>
#include <stdio.h>
#include <string.h>

#define NAME "/myMsgQueue"
#define MAX_MSGSIZE 1000

int main() {
    /* 設定 queue 屬性：最多 10 則訊息，每則最多 MAX_MSGSIZE bytes */
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MSGSIZE;
    attr.mq_curmsgs = 0;

    /* 建立（或開啟）message queue，只寫 */
    mqd_t mqd = mq_open(NAME, O_WRONLY | O_CREAT, 0644, &attr);
    if (mqd == (mqd_t)-1) {
        perror("Failed to open or create the message queue");
        return 1;
    }

    printf("Send to receiver:\n");
    while (1) {
        char buffer[MAX_MSGSIZE];
        memset(buffer, '\0', sizeof(buffer));

        /* 讓使用者輸入字串 */
        printf("> ");
        fflush(stdout);
        if (fgets(buffer, MAX_MSGSIZE, stdin) == NULL)
            strcpy(buffer, "exit");          // 遇到 EOF (Ctrl+D) 也當作 exit

        buffer[strcspn(buffer, "\n")] = '\0'; // 去掉 fgets 讀進來的換行

        /* 送出訊息（包含結尾的 '\0'） */
        if (mq_send(mqd, buffer, strlen(buffer) + 1, 0) == -1) {
            perror("Failed to send a message");
            return 1;
        }

        /* 輸入 exit 就結束（exit 也有送給 receiver，讓它知道要結束） */
        if (strcmp(buffer, "exit") == 0)
            break;
    }

    /* sender 只關閉，不 unlink；由 receiver 負責刪除 queue */
    if (mq_close(mqd) == -1) {
        perror("Failed to close the message queue");
        return 1;
    }
    return 0;
}
