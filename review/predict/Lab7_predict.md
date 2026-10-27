# Lab 7 上機考猜題：IPC（Message Queue + Shared Memory）

> 依據：Lab7-1.pdf（27 頁，Message Queue）、Lab7-2.pdf（30 頁，Shared Memory）逐頁看過（兩份都是純圖片 PDF），加上 `practice/Lab7_ex1`、`ex2` 參考答案。
> 頁碼寫法：**7-1 p.13** = Lab7-1.pdf 第 13 頁；**7-2 p.18** = Lab7-2.pdf 第 18 頁。
> 每一題的程式都在本機用 `gcc 13.3 (Ubuntu 24.04)` 加 `-Wall -lrt` 實際編譯執行過，用「receiver 背景執行 + 管線餵 sender」模擬兩個終端機。queue / shm 名稱都加 `_lab7p` 後綴，測完都已 unlink，`/dev/mqueue`、`/dev/shm` 沒有殘留。
> （環境備註：容器一開始沒有掛 `/dev/mqueue`，但 `mq_open` 等系統呼叫本身可用；我另外 `mount -t mqueue none /dev/mqueue` 方便查看殘留 queue。上機時 Ubuntu 預設就有 `/dev/mqueue`。）

---

## 0. 這個 Lab 專屬的出題邏輯（補充共通邏輯）

### 0.1 簡報教了、但 Exercise 沒用到的東西（＝最可能被拿來變形的點）

| 簡報內容 | 頁碼 | Exercise 有沒有用 | 可以怎麼考 |
|---|---|---|---|
| `mq_send(..., unsigned int msg_prio)` / `mq_receive(..., unsigned int *msg_prio)` | 7-1 p.13–14 | **沒有**，簡報明講「Here, we ignore the message priority」→ 傳 0 / NULL | 依 priority 接收、印出 priority（第 4 題） |
| `struct mq_attr` 4 個欄位、`mq_getattr()` / `mq_setattr()`（寫在欄位註解中） | 7-1 p.10 | 範例傳 `NULL` 用預設 | 印出 `mq_maxmsg / mq_msgsize / mq_curmsgs`（第 4 題） |
| `O_NONBLOCK`（oflag 表最後一列；也是 `mq_flags` 的值） | 7-1 p.8、p.10 | 沒有 | queue 空了就結束，不卡住（第 4 題） |
| `O_EXCL`（與 O_CREAT 併用 = 只能新建） | 7-1 p.8、7-2 p.12 | 沒有 | 坑：舊 queue 殘留時 `File exists` |
| 「receiver 的 buffer 必須 ≥ mq_msgsize」 | 7-1 p.14 | 有，但投影片範例本身是**錯的**（buffer 1000 卻傳 8192） | 坑題 / 口頭問 |
| `mq_close` 不會刪 queue；`mq_unlink` 才刪（refcount 歸零才真正銷毀） | 7-1 p.11–12 | 有（receiver close & unlink） | 誰負責 unlink、兩條 queue 都要 unlink |
| shm 的 **kernel persistence**、物件在 `/dev/shm` | 7-2 p.7 | 間接 | 「驗證觀念」題（第 6 題） |
| `MAP_PRIVATE` vs `MAP_SHARED`（簡報用**紅框**標出來） | 7-2 p.18 | 只用 MAP_SHARED | 「驗證觀念」題（第 6 題） |
| `PROT_READ / PROT_WRITE / PROT_EXEC / PROT_NONE` | 7-2 p.17 | 只用 READ/WRITE | consumer 用 PROT_READ 卻寫入 → Segmentation fault |
| `O_TRUNC`、`O_RDONLY` vs `O_RDWR` | 7-2 p.12 | 部分 | consumer 用 O_RDONLY 就只能 PROT_READ |
| `ftruncate` 設大小、`munmap` 長度 | 7-2 p.19–20 | 有（固定 1024） | 改存 struct → 大小要用 `sizeof(struct ...)`（第 2、3 題） |
| 同步要用 semaphore（**Lab 15 才教**） | 7-2 p.8 | 無 | **不會考 semaphore**；若要「通知」只能用本 Lab 的 message queue（第 5 題） |

### 0.2 Lab 7 特有的判斷
1. **7-1 Exercise 已經是「反覆輸入直到 exit」**，所以變形方向是：**改成雙向**（加第二條 queue 回傳結果）、**用上被忽略的 priority**、**用 mq_attr/getattr**。
2. **7-2 Exercise 只寫一次、讀一次**，最自然的變形是：**寫多筆 / 寫 struct / 寫數字讓 consumer 計算**。shared memory 沒有同步機制（semaphore 是 Lab 15），所以題目最可能維持「producer 先跑完、consumer 再跑」的順序。
3. 兩份簡報同屬 Lab 7、Reference 頁也互相列出（7-2 p.29 同時列 shm 與 message queue 連結），**「結合兩者」是可能的加分題**：shm 放資料、mq 當通知。
4. Exercise 提示的 `fgets`、`strcmp`（7-1 p.24）一定要會；`sscanf`/`atoi` 屬於基本 C，可以用。
5. 簡報範例的 bug 要知道（考官可能故意給你範例改）：
   - 7-1 p.18 receiver：`char buffer[1000]` 卻 `mq_receive(mqd, buffer, 8192, NULL)` → 謊報 buffer 大小，訊息超過 1000 會溢位。正確：buffer 開到 ≥ mq_msgsize（或建 queue 時就指定 attr）。
   - 7-2 p.23 producer：`sprintf(ptr, "%s", buffer)` 而 `buffer` 沒初始化 → 寫進去的是垃圾。
   - 7-2 p.24 consumer：`printf("%p\n", map_addr)` 印的是**位址**不是內容 → 應改 `%s`。

### 0.3 可能性排序總表

| # | 題目 | 主題 | 可能性 |
|---|---|---|---|
| 1 | 雙向 Message Queue：計算機 client / server | MQ | **高** |
| 2 | Shared Memory 寫入多筆訊息（struct）直到 exit | SHM | **高** |
| 3 | Shared Memory 傳整數陣列，consumer 計算 sum/avg/max/min | SHM | 中高 |
| 4 | 依 Priority 接收 + mq_getattr + O_NONBLOCK | MQ | 中 |
| 5 | 結合兩者：shm 放資料、message queue 當通知 | MQ + SHM | 中 |
| 6 | 驗證觀念：kernel persistence 與 MAP_PRIVATE vs MAP_SHARED | SHM | 中低 |

---

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

---

## 第 2 題（可能性：高）Shared Memory 寫入多筆訊息直到 exit

### 題目
- `producer.c`：建立 shared memory，讓使用者**反覆輸入訊息直到 `exit`**（最多 10 筆），全部存進 shared memory，結束時印出寫入筆數。
- `consumer.c`：讀出 shared memory 中**所有**訊息，加上編號印出，最後 `munmap`、`close`、`shm_unlink`。

終端機 A：
```
$ ./producer
Enter messages (type exit to finish):
> hello
> OS lab 7
> shared memory
> exit
3 message(s) written to shared memory.
```
終端機 B（producer 結束後執行）：
```
$ ./consumer
Read 3 message(s) from shared memory:
[1] hello
[2] OS lab 7
[3] shared memory
```

### 變形自哪裡 / 考什麼
- 變形自 **7-2 Exercise 2（p.26–28）**：單筆 → 多筆，並把 7-1 Exercise 的「反覆輸入直到 exit」搬過來（兩個 exercise 串在一起）。
- 考：`shm_open → ftruncate → mmap → munmap → close → shm_unlink` 全流程（7-2 p.10–21）；shared memory 是一塊**記憶體**，可以直接當 struct 指標用；`ftruncate` 大小要夠。

### 為什麼可能性高
7-2 Exercise 只寫一筆太簡單，「寫多筆」是最直接的加深；而且需要「筆數」這種額外資訊，自然會用到 struct，考你懂 shm 只是一塊 bytes。

### 參考答案
`producer.c`
```c
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>     // ftruncate() & close()

#define NAME "/myShareMemory_lab7p"
#define MAX_MSG 10
#define MSG_LEN 100

/* shared memory 裡放的資料格式：兩邊必須「一模一樣」 */
struct shm_data {
    int count;                   // 已寫入幾筆
    char msg[MAX_MSG][MSG_LEN];  // 每筆訊息
};

int main() {
    int fd = shm_open(NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("Failed to open or create the shared memory");
        return 1;
    }
    if (ftruncate(fd, sizeof(struct shm_data)) == -1) {
        perror("Failed to resize");
        return 1;
    }
    struct shm_data *data = mmap(NULL, sizeof(struct shm_data),
                                 PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        perror("Failed to create the map");
        return 1;
    }

    data->count = 0;
    printf("Enter messages (type exit to finish):\n");
    while (data->count < MAX_MSG) {
        char buffer[MSG_LEN];
        printf("> ");
        fflush(stdout);
        if (fgets(buffer, MSG_LEN, stdin) == NULL)
            break;
        buffer[strcspn(buffer, "\n")] = '\0';
        if (strcmp(buffer, "exit") == 0)
            break;
        strcpy(data->msg[data->count], buffer);  // 寫進 shared memory
        data->count++;
    }
    printf("%d message(s) written to shared memory.\n", data->count);

    munmap(data, sizeof(struct shm_data));
    close(fd);        // 不 unlink：資料要留給 consumer
    return 0;
}
```
`consumer.c`
```c
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

#define NAME "/myShareMemory_lab7p"
#define MAX_MSG 10
#define MSG_LEN 100

struct shm_data {
    int count;
    char msg[MAX_MSG][MSG_LEN];
};

int main() {
    int fd = shm_open(NAME, O_RDONLY, 0666);   // 不加 O_CREAT：producer 要先跑
    if (fd == -1) {
        perror("Failed to open the shared memory");
        return 1;
    }
    struct shm_data *data = mmap(NULL, sizeof(struct shm_data),
                                 PROT_READ, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        perror("Failed to read the map");
        return 1;
    }

    printf("Read %d message(s) from shared memory:\n", data->count);
    for (int i = 0; i < data->count; i++)
        printf("[%d] %s\n", i + 1, data->msg[i]);

    munmap(data, sizeof(struct shm_data));
    close(fd);
    if (shm_unlink(NAME) == -1) {               // 最後讀完的人負責刪除
        perror("Failed to unlink the shared memory");
        return 1;
    }
    return 0;
}
```
編譯：`gcc -o producer producer.c -lrt`、`gcc -o consumer consumer.c -lrt`

### ✅ 已驗證
```
$ printf 'hello\nOS lab 7\nshared memory\nexit\n' | ./producer
Enter messages (type exit to finish):
> > > > 3 message(s) written to shared memory.
$ ls /dev/shm
myShareMemory_lab7p
$ ./consumer
Read 3 message(s) from shared memory:
[1] hello
[2] OS lab 7
[3] shared memory
$ ls /dev/shm            # 已被 shm_unlink 刪掉
$ ./consumer             # 再跑一次
Failed to open the shared memory: No such file or directory
```

### 容易踩的坑
- **struct 定義兩邊必須完全一樣**（欄位順序、陣列大小），否則 consumer 讀到錯位的資料。
- `ftruncate` 和 `mmap`、`munmap` 的長度都用 `sizeof(struct shm_data)`；`ftruncate` 太小，存取超出範圍會 `Bus error`。
- 不能在 struct 裡放 `char *`（指標指向的是 producer 自己的位址空間，consumer 看不到），要放 `char msg[10][100]` 這種**真正的陣列**。
- producer **不要 shm_unlink**，否則 consumer 打不開；由 consumer 讀完再 unlink。
- consumer 用 `O_RDONLY` 開，`mmap` 就只能 `PROT_READ`，寫 `PROT_READ|PROT_WRITE` 會 `Permission denied`。
- 要先執行 producer 再執行 consumer（沒有同步機制，semaphore 是 Lab 15）。

---

## 第 3 題（可能性：中高）Shared Memory 傳整數陣列，consumer 計算

### 題目
- `producer.c`：使用者反覆輸入整數直到 `exit`，把**整數**（不是字串）存進 shared memory。
- `consumer.c`：讀出所有整數，印出 sum、avg（小數兩位）、max、min，最後清理 shared memory。

終端機 A：
```
$ ./producer
Enter integers (type exit to finish):
> 12
> 7
> -3
> 25
> 9
> exit
5 number(s) written.
```
終端機 B：
```
$ ./consumer
Numbers: 12 7 -3 25 9
sum = 50, avg = 10.00, max = 25, min = -3
```

### 變形自哪裡 / 考什麼
- 變形自 **7-2 Exercise 2**：資料型態從字串改成 int 陣列，consumer 從「印出」改成「計算」。
- 考：shared memory 可以存任何型態（mmap 回傳 `void *`，7-2 p.14）；`ftruncate`（p.19）大小計算。
- 也可以出成 MQ 版（sender 送數字、receiver 計算）—— 解法就是第 1 題的單向版。

### 為什麼可能性中高
「傳數字讓對方算」是 IPC 題最常見的變形，但跟第 2 題結構幾乎一樣，兩者擇一出的機率高，所以排在第 2 題後面。

### 參考答案
`producer.c`
```c
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#define NAME "/numShm_lab7p"
#define MAX_N 100

struct shm_data {
    int n;
    int num[MAX_N];
};

int main() {
    int fd = shm_open(NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) { perror("shm_open"); return 1; }
    if (ftruncate(fd, sizeof(struct shm_data)) == -1) { perror("ftruncate"); return 1; }
    struct shm_data *data = mmap(NULL, sizeof(struct shm_data),
                                 PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) { perror("mmap"); return 1; }

    data->n = 0;
    printf("Enter integers (type exit to finish):\n");
    char buffer[64];
    while (data->n < MAX_N) {
        printf("> ");
        fflush(stdout);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
            break;
        buffer[strcspn(buffer, "\n")] = '\0';
        if (strcmp(buffer, "exit") == 0)
            break;
        data->num[data->n++] = atoi(buffer);   // 存「整數」，不是字串
    }
    printf("%d number(s) written.\n", data->n);

    munmap(data, sizeof(struct shm_data));
    close(fd);
    return 0;
}
```
`consumer.c`
```c
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

#define NAME "/numShm_lab7p"
#define MAX_N 100

struct shm_data {
    int n;
    int num[MAX_N];
};

int main() {
    int fd = shm_open(NAME, O_RDONLY, 0666);
    if (fd == -1) { perror("shm_open"); return 1; }
    struct shm_data *data = mmap(NULL, sizeof(struct shm_data),
                                 PROT_READ, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) { perror("mmap"); return 1; }

    if (data->n == 0) {
        printf("no data.\n");
    } else {
        int sum = 0, max = data->num[0], min = data->num[0];
        printf("Numbers:");
        for (int i = 0; i < data->n; i++) {
            printf(" %d", data->num[i]);
            sum += data->num[i];
            if (data->num[i] > max) max = data->num[i];
            if (data->num[i] < min) min = data->num[i];
        }
        printf("\nsum = %d, avg = %.2f, max = %d, min = %d\n",
               sum, (double)sum / data->n, max, min);
    }

    munmap(data, sizeof(struct shm_data));
    close(fd);
    shm_unlink(NAME);
    return 0;
}
```

### ✅ 已驗證
```
$ printf '12\n7\n-3\n25\n9\nexit\n' | ./producer
Enter integers (type exit to finish):
> > > > > > 5 number(s) written.
--- consumer terminal ---
$ ./consumer
Numbers: 12 7 -3 25 9
sum = 50, avg = 10.00, max = 25, min = -3
$ ls /dev/shm      # 無 numShm_lab7p，已清乾淨
```

### 容易踩的坑
- avg 要轉 `double`：`(double)sum / n`，否則整數除法。
- `n == 0` 時不能除以 0、也不能拿 `num[0]` 當 max 初值。
- 若題目要求「存字串，consumer 再 atoi」也可以，但要注意每筆字串的長度固定（二維陣列）。
- 若出成 MQ 版：`mq_send` 的 `msg_ptr` 型別是 `const char *`，要送 int 就 `(char *)&x, sizeof(int)`，或者乾脆用 `sprintf` 轉字串送（較安全，推薦）。

---

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

---

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

---

## 第 6 題（可能性：中低）驗證觀念：kernel persistence 與 MAP_PRIVATE vs MAP_SHARED

### 題目
1. `writer.c`：建立 shared memory，寫入 `"original data"` 後直接結束（**不 unlink**）。用 `ls /dev/shm` 證明 process 結束後物件仍存在。
2. `reader.c`：先用 `MAP_PRIVATE`（PROT_READ|PROT_WRITE）映射並修改內容，再用 `MAP_SHARED` 映射同一個物件，印出內容，證明 MAP_PRIVATE 的修改**不會**影響到 shared memory；接著用 MAP_SHARED 修改。再執行一次 reader 證明 MAP_SHARED 的修改有留下來。
3. `cleaner.c`：印出最終內容並 `shm_unlink`，之後 `ls /dev/shm` 看不到物件。

輸出範例：
```
$ ./writer
writer: wrote "original data" and exit (no shm_unlink)
$ ls /dev/shm
persistShm_lab7p
$ ./reader
[PRIVATE] before: original data
[PRIVATE] after : modified by MAP_PRIVATE
[SHARED ] content: original data
[SHARED ] after : modified by MAP_SHARED
$ ./reader
[PRIVATE] before: modified by MAP_SHARED
[PRIVATE] after : modified by MAP_PRIVATE
[SHARED ] content: modified by MAP_SHARED
[SHARED ] after : modified by MAP_SHARED
$ ./cleaner
cleaner: final content = modified by MAP_SHARED
cleaner: shm_unlink done
$ ls /dev/shm
```

### 變形自哪裡 / 考什麼
- 7-2 p.7：objects 在 `/dev/shm`；**kernel persistence**——沒有 process 開著也會存在，直到關機或 unlink。
- 7-2 p.18：`MAP_PRIVATE`「Modifications to mapped data are private」、`MAP_SHARED`「visible to other processes」——**簡報用紅框圈起來**。
- 符合共通邏輯 2「Exercise 常用來驗證一個觀念」。

### 為什麼可能性中低
是純觀念驗證，程式量小；比較可能以「口頭問答 / 修改一個參數觀察結果」形式出現，而不是完整 50 分題。但紅框內容值得準備。

### 參考答案
`writer.c`
```c
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define NAME "/persistShm_lab7p"
#define SMSIZE 1024

int main() {
    int fd = shm_open(NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) { perror("shm_open"); return 1; }
    if (ftruncate(fd, SMSIZE) == -1) { perror("ftruncate"); return 1; }
    char *ptr = mmap(NULL, SMSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) { perror("mmap"); return 1; }

    strcpy(ptr, "original data");
    printf("writer: wrote \"%s\" and exit (no shm_unlink)\n", ptr);

    munmap(ptr, SMSIZE);
    close(fd);
    return 0;      // process 結束了，但 shared memory 物件還在（kernel persistence）
}
```
`reader.c`
```c
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define NAME "/persistShm_lab7p"
#define SMSIZE 1024

int main() {
    int fd = shm_open(NAME, O_RDWR, 0666);
    if (fd == -1) { perror("shm_open"); return 1; }

    /* (1) MAP_PRIVATE：改了只影響自己（copy-on-write），不會寫回 shared memory */
    char *priv = mmap(NULL, SMSIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (priv == MAP_FAILED) { perror("mmap private"); return 1; }
    printf("[PRIVATE] before: %s\n", priv);
    strcpy(priv, "modified by MAP_PRIVATE");
    printf("[PRIVATE] after : %s\n", priv);

    /* (2) MAP_SHARED：看到的是真正 shared memory 的內容 */
    char *shared = mmap(NULL, SMSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared == MAP_FAILED) { perror("mmap shared"); return 1; }
    printf("[SHARED ] content: %s\n", shared);

    strcpy(shared, "modified by MAP_SHARED");
    printf("[SHARED ] after : %s\n", shared);

    munmap(priv, SMSIZE);
    munmap(shared, SMSIZE);
    close(fd);
    return 0;
}
```
`cleaner.c`
```c
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

#define NAME "/persistShm_lab7p"
#define SMSIZE 1024

int main() {
    int fd = shm_open(NAME, O_RDONLY, 0666);
    if (fd == -1) { perror("shm_open"); return 1; }
    char *ptr = mmap(NULL, SMSIZE, PROT_READ, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) { perror("mmap"); return 1; }
    printf("cleaner: final content = %s\n", ptr);
    munmap(ptr, SMSIZE);
    close(fd);
    if (shm_unlink(NAME) == -1) { perror("shm_unlink"); return 1; }
    printf("cleaner: shm_unlink done\n");
    return 0;
}
```

### ✅ 已驗證
```
writer: wrote "original data" and exit (no shm_unlink)
$ ls /dev/shm
persistShm_lab7p
--- reader terminal ---
[PRIVATE] before: original data
[PRIVATE] after : modified by MAP_PRIVATE
[SHARED ] content: original data              ← MAP_PRIVATE 的修改沒有寫回
[SHARED ] after : modified by MAP_SHARED
--- reader again ---
[PRIVATE] before: modified by MAP_SHARED      ← MAP_SHARED 的修改有留下
...
--- cleaner ---
cleaner: final content = modified by MAP_SHARED
cleaner: shm_unlink done
$ ls /dev/shm                                  ← 已消失
$ ./reader
shm_open: No such file or directory
```

### 容易踩的坑
- `MAP_PRIVATE` + `PROT_WRITE` 可以搭配 `O_RDONLY` 的 fd（copy-on-write）；但 `MAP_SHARED` + `PROT_WRITE` 一定要 `O_RDWR` 開，否則 `mmap` 回 `Permission denied`（兩者皆已實測）。
- 用 `PROT_READ` 映射卻去寫 → **Segmentation fault**。
- 同理，message queue 也有 kernel persistence：第 4 題中 sender 結束後 `/dev/mqueue/prioQueue_lab7p` 仍在、`QSIZE:60`。

---

## 共通易錯總整理

| 坑 | 症狀 | 解法 |
|---|---|---|
| 忘了 `-lrt` | 舊版 glibc（< 2.34）連結錯誤 `undefined reference to mq_open / shm_open` | 一律 `gcc x.c -o x -lrt`（7-2 p.27）。本機 glibc 2.39 不加也能過，**但考場照樣加** |
| `mq_receive` buffer < mq_msgsize | `Message too long` | buffer ≥ mq_msgsize；NULL attr 預設 8192 |
| 舊 queue / shm 殘留 | attr 被忽略、收到上次的舊訊息、O_EXCL 回 `File exists` | 結束時 unlink；考試前 `ls /dev/mqueue /dev/shm` 檢查、`rm` 掉 |
| `mq_open` 判斷錯誤 | — | `if (mqd == (mqd_t)-1)` |
| `mmap` 判斷錯誤 | — | `if (ptr == MAP_FAILED)`，**不是** `NULL` |
| `fgets` 留下 `\n` | `strcmp(buf, "exit")` 永遠不等於 0 | `buf[strcspn(buf, "\n")] = '\0';` |
| `mq_send` 長度用 `sizeof(buffer)` | 可以動但送一堆多餘 bytes；用 `strlen(buffer)` 則沒有 `\0` | `strlen(buffer) + 1` |
| producer 自己 shm_unlink | consumer `No such file or directory` | 由最後讀的人 unlink |
| shm 沒 ftruncate | 存取時 `Bus error`（已實測） | `shm_open` 後立刻 `ftruncate(fd, size)` |
| printf 沒換行、輸出順序怪 | 提示字先沒出來 | `fflush(stdout)` |
| mq 名字沒有 `/` 開頭 | `mq_open` 回 `Invalid argument`（已實測） | 名字一律寫 `"/name"`（shm 也照這格式） |

---

## 考前 5 分鐘速查

### Message Queue（7-1）
```c
#include <fcntl.h>      // O_CREAT, O_RDONLY, O_WRONLY, O_RDWR, O_EXCL, O_NONBLOCK
#include <sys/stat.h>   // 0644 / S_IRUSR ...
#include <mqueue.h>     // mqd_t, mq_*
#include <stdio.h>
#include <string.h>     // strlen, strcmp, strcspn

struct mq_attr attr = {0};
attr.mq_maxmsg  = 10;          // 只需設這兩個（≤ 10）
attr.mq_msgsize = 1000;

mqd_t mqd = mq_open("/name", O_WRONLY | O_CREAT, 0644, &attr);   // 失敗 == (mqd_t)-1
mq_send(mqd, buf, strlen(buf) + 1, prio);                         // 0 / -1
ssize_t n = mq_receive(mqd, buf, attr.mq_msgsize, &prio_or_NULL); // buf >= mq_msgsize
mq_getattr(mqd, &attr);       // 取 mq_maxmsg / mq_msgsize / mq_curmsgs
mq_close(mqd);                // 只關，不刪
mq_unlink("/name");           // 刪名字（最後一個用的人做）
```
順序：**mq_open → (mq_send / mq_receive 迴圈) → mq_close → mq_unlink**
規則：priority 大先出、同 priority FIFO；`O_NONBLOCK` 空 queue → -1 / `EAGAIN`。

### Shared Memory（7-2）
```c
#include <fcntl.h>      // O_CREAT, O_RDWR, O_RDONLY, O_EXCL, O_TRUNC
#include <sys/stat.h>   // mode
#include <sys/mman.h>   // shm_open, mmap, munmap, shm_unlink, PROT_*, MAP_*
#include <unistd.h>     // ftruncate, close

int fd = shm_open("/name", O_CREAT | O_RDWR, 0666);        // 失敗 -1
ftruncate(fd, SIZE);                                        // 只有建立者需要
char *p = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);  // 失敗 MAP_FAILED
/* ... 直接用 p 讀寫 ... */
munmap(p, SIZE);
close(fd);
shm_unlink("/name");                                        // 最後讀的人做
```
- producer：**shm_open(O_CREAT|O_RDWR) → ftruncate → mmap(PROT_READ|PROT_WRITE, MAP_SHARED) → 寫 → munmap → close**（不 unlink）
- consumer：**shm_open(O_RDONLY) → mmap(PROT_READ, MAP_SHARED) → 讀 → munmap → close → shm_unlink**
- 物件在 `/dev/shm/`，queue 在 `/dev/mqueue/`；兩者都是 kernel persistence。

### 編譯
```
gcc -o sender sender.c -lrt
gcc -o receiver receiver.c -lrt
gcc -o producer producer.c -lrt
gcc -o consumer consumer.c -lrt
```
