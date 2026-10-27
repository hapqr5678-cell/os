# Lab7_g1

> 來源:review/predict/Lab7_predict.md

## 第 1 題（可能性：高）雙向 Message Queue：計算機 client / server

### 題目
寫兩支程式：
- `client.c`：建立兩個 message queue（請求用、回應用）。使用者反覆輸入 `a op b`（op 為 `+ - * /`），送給 server，然後**等 server 回傳結果**並印出；輸入 `exit` 時把 `exit` 也送給 server，然後結束。
- `server.c`：反覆接收運算式，計算後把結果送回 client，並在自己的畫面印出；收到 `exit` 時結束，**close 並 unlink 兩個 queue**。

輸出範例（終端機 A：server 先執行）：
```
$ ./server
Server is waiting...
received: 3 + 5 -> 8
received: 10 * 4 -> 40
received: 7 / 0 -> invalid input
received: 100 - 1 -> 99
Server exit.
```
終端機 B：
```
$ ./client
Enter expression (e.g. 3 + 5), type exit to quit:
> 3 + 5
result: 8
> 10 * 4
result: 40
> 7 / 0
result: invalid input
> 100 - 1
result: 99
> exit
```

### 變形自哪裡 / 考什麼
- 變形自 **7-1 Exercise（p.22–24）**：一樣是「fgets 反覆輸入 + strcmp 判斷 exit」，但從單向變成**雙向**。
- 考：一條 message queue 是單向的（7-1 p.3 圖），雙向要開**兩條**；`O_RDONLY / O_WRONLY / O_CREAT`（p.8）；`mq_send / mq_receive`（p.13–14）；`mq_close / mq_unlink`（p.11–12）。
- 也是「傳數字讓對方計算後回傳」的版本：訊息是字串，用 `sscanf` 解析。

### 為什麼可能性高
Exercise 本身已經是 sender→receiver 的迴圈，最小幅度又有變化的改法就是「讓 receiver 回話」。只用簡報 5 個 API，難度適中，很適合當 50 分題。

### 參考答案
`client.c`
```c
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
```
`server.c`
```c
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
```
編譯：`gcc -o client client.c -lrt`、`gcc -o server server.c -lrt`

### ✅ 已驗證
指令：`./server > server.log &` 後 `printf '3 + 5\n10 * 4\n7 / 0\n100 - 1\nexit\n' | ./client`
```
Enter expression (e.g. 3 + 5), type exit to quit:
> result: 8
> result: 40
> result: invalid input
> result: 99
> --- server terminal ---
Server is waiting...
received: 3 + 5 -> 8
received: 10 * 4 -> 40
received: 7 / 0 -> invalid input
received: 100 - 1 -> 99
Server exit.
```
（管線輸入不會回顯，所以 client 那邊看不到使用者打的字，實際在終端機打字會看到。）結束後 `/dev/mqueue` 是空的。

### 容易踩的坑
- **只開一條 queue 做雙向**：client 自己 send 完馬上 receive，可能把自己剛送的訊息收回來。雙向一定兩條。
- 兩邊開 queue 的**讀寫方向要相反**（client：req 寫、res 讀；server：req 讀、res 寫），寫錯會 `Bad file descriptor`。
- 兩邊都加 `O_CREAT` 且帶**相同 attr**，誰先跑都可以；如果只有一邊 O_CREAT，另一邊先跑會 `No such file or directory`。
- server 要 unlink **兩條**；只 unlink 一條，下次執行舊 queue 還在（裡面可能殘留舊訊息）。
- client 送 `exit` 之後**不要再 mq_receive**，否則 server 不回話會卡死。
- `mq_send` 長度用 `strlen(buffer) + 1`，把 `'\0'` 一起送（7-1 p.17 紅框）。
