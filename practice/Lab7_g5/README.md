# Lab7_g5

> 來源:review/predict/Lab7_predict.md

## 第 5 題（可能性：中）結合兩者：shm 放資料、message queue 當通知

### 題目
Exercise 2 的 consumer 必須等 producer 寫完才能手動執行。請改成：**consumer 可以先執行並等待**，producer 每輸入一筆訊息就寫進 shared memory，然後用 message queue 通知 consumer「資料好了」；consumer 收到通知才去讀 shared memory 並印出，讀完再回覆 producer 可以寫下一筆。輸入 `exit` 時兩邊都結束，由 consumer 清除所有 IPC 物件。

終端機 A（consumer 先執行）：
```
$ ./consumer
Waiting for producer...
Read from shared memory: hello
Read from shared memory: second message
Read from shared memory: bye bye
Producer finished.
```
終端機 B：
```
$ ./producer
Enter the message: hello
Enter the message: second message
Enter the message: bye bye
Enter the message: exit
```

### 變形自哪裡 / 考什麼
- 7-1 + 7-2 兩個 Exercise 結合；呼應 7-2 p.8「多個 process 同時存取 shared memory 需要同步」——但 semaphore 是 Lab 15，**本 Lab 能用的同步工具只有 message queue 的阻塞 `mq_receive`**。
- 考：shm 沒有通知機制；`mq_receive` 在 queue 空時會阻塞（預設，非 O_NONBLOCK）可以拿來「等」。

### 為什麼可能性中
需要同時寫 6 個 API 群組，對 50 分上機題略重；但「為什麼 consumer 要後跑？」是很自然的追問，且只用簡報 API 就能解。可能以「加分題」或「口頭問答」出現。

### 參考答案
`producer.c`
```c
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
```
`consumer.c`
```c
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
```

### ✅ 已驗證
指令：`./consumer > consumer.log &`（先跑）後 `printf 'hello\nsecond message\nbye bye\nexit\n' | ./producer`
```
Enter the message: Enter the message: Enter the message: Enter the message:
--- consumer terminal ---
Waiting for producer...
Read from shared memory: hello
Read from shared memory: second message
Read from shared memory: bye bye
Producer finished.
--- leftovers:
/dev/mqueue:          ← 空
/dev/shm:             ← 只剩系統本來的檔案，dataShm_lab7p 已刪
```

### 容易踩的坑
- **沒有 ack 會出事**：producer 只通知不等回覆，就可能在 consumer 還沒印之前把下一筆蓋掉（shm 只有一份）。所以要第二條 queue 回覆「done」。
- consumer 先跑時 shm 可能還不存在 → 先等第一個通知再 `shm_open`（或兩邊都 `O_CREAT|O_RDWR` + `ftruncate`）。
- 兩條 queue + 一個 shm，**三個名字都要 unlink**。
- `exit` 也要通知 consumer，否則 consumer 永遠卡在 `mq_receive`。
