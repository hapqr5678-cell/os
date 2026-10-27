# Lab7_g4

> 來源:review/predict/Lab7_predict.md

## 第 4 題（可能性：中）依 Priority 接收 + mq_getattr + O_NONBLOCK

### 題目
- `sender.c`：使用者反覆輸入 `<priority> <message>` 直到 `exit`，以指定 priority 送進 queue；結束時**只 close 不 unlink**。
- `receiver.c`：**在 sender 結束之後才執行**。先用 `mq_getattr` 印出 queue 的 `maxmsg / msgsize / curmsgs`，再用**非阻塞**方式把 queue 中訊息全部收完並印出各自的 priority，queue 空了就結束，最後 close & unlink。

終端機 A：
```
$ ./sender
Input format: <priority> <message>, type exit to quit
> 1 low task
> 5 urgent task
> 3 normal task A
> 3 normal task B
> 9 fire alarm
> exit
```
終端機 B（之後執行）：
```
$ ./receiver
maxmsg=10, msgsize=256, curmsgs=5
received (prio 9): fire alarm
received (prio 5): urgent task
received (prio 3): normal task A
received (prio 3): normal task B
received (prio 1): low task
queue is empty.
```

### 變形自哪裡 / 考什麼
- 7-1 p.13–14 的 `msg_prio` 參數（簡報說「這裡先忽略」→ 正好是出題空間）。
- 7-1 p.10 `struct mq_attr` 與 `mq_getattr()`、`mq_curmsgs`。
- 7-1 p.8 `O_NONBLOCK`：queue 空時 `mq_receive` 回傳 -1、`errno == EAGAIN`，而不是卡住。
- 7-1 p.11 `mq_close` 不刪 queue → sender 結束後訊息仍在（驗證觀念）。
- 觀念：**priority 大的先出來；同 priority 依 FIFO**（task A 在 task B 前）。

### 為什麼可能性中
簡報把 priority 與 mq_attr 各給了一整頁，但明說「ignore」，Exercise 沒用到，是典型「簡報有教、exercise 沒考」的補考點。只是 `errno/EAGAIN` 簡報沒直接寫，所以考官也可能改成「先 getattr 拿 curmsgs，再收 curmsgs 次」（不需 O_NONBLOCK，見下方替代寫法）。

### 參考答案
`sender.c`
```c
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
```
`receiver.c`
```c
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
```
**替代寫法（不用 O_NONBLOCK / errno）**：`mq_open(NAME, O_RDONLY)` 後 `mq_getattr` 取 `attr.mq_curmsgs`，`for (long i = 0; i < attr.mq_curmsgs; i++) mq_receive(...)`。

### ✅ 已驗證
```
$ printf '1 low task\n5 urgent task\n3 normal task A\n3 normal task B\n9 fire alarm\nexit\n' | ./sender
$ cat /dev/mqueue/prioQueue_lab7p        # sender 已結束，queue 還在（mq_close 不刪 queue）
QSIZE:60         NOTIFY:0     SIGNO:0     NOTIFY_PID:0
$ ./receiver
maxmsg=10, msgsize=256, curmsgs=5
received (prio 9): fire alarm
received (prio 5): urgent task
received (prio 3): normal task A
received (prio 3): normal task B
received (prio 1): low task
queue is empty.
$ ls /dev/mqueue                          # 已 unlink，空的
```
另外驗證了兩個坑（`q3/pit.c`、`q3/pit2.c`）：
```
default maxmsg=10 msgsize=8192
mq_receive with 1000-byte buffer: Message too long        ← buffer < mq_msgsize
asked msgsize=1000, actual msgsize=8192                     ← 舊 queue 殘留，attr 被忽略
mq_open with O_EXCL: File exists                           ← O_CREAT|O_EXCL 遇到已存在的 queue
```

### 容易踩的坑
- `mq_receive` 第 4 個參數是 `unsigned int *`，要傳 `&prio`（不是 `prio`）；`mq_send` 第 4 個是值。
- **buffer 必須 ≥ mq_msgsize**（7-1 p.14），否則 `Message too long (EMSGSIZE)`；建 queue 時傳 `NULL` 預設是 8192，最保險的是 `mq_getattr` 後 `malloc(attr.mq_msgsize)`。
- `mq_maxmsg = 10`：sender 送超過 10 則而沒人收時，**阻塞模式的 sender 會卡住**（Linux 預設上限 `/proc/sys/fs/mqueue/msg_max` = 10，一般使用者設更大會 `Invalid argument`；root 不受限）。
- **舊 queue 殘留**：上次沒 unlink，這次 `mq_open(..., O_CREAT, ..., &attr)` 的 attr **會被忽略**，沿用舊的 msgsize/舊訊息 → 測試前可以 `ls /dev/mqueue`、`rm /dev/mqueue/xxx` 清掉。
- receiver 不加 `O_CREAT` → 如果 sender 沒先跑會 `No such file or directory`（題目本來就要求 sender 先跑）。
- 要 `#include <errno.h>` 才能用 `errno`、`EAGAIN`。
