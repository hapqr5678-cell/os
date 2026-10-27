# Lab 7 複習：IPC — Message Queue（7-1）＋ Shared Memory（7-2）

> 來源：Lab7-1.pdf（27 頁）、Lab7-2.pdf（30 頁）——兩份 PDF 都是**圖片型**投影片，沒有文字層，內容是將頁面圖片逐張檢視後整理的。
> 學生程式：practice/lab7/ex1/（sender.c、receiver.c）、practice/lab7/ex2/（producer.c、consumer.c）以及編好的執行檔。

---

## 1. 這個 LAB 在學什麼

學兩種讓**不相關的 process 交換資料**的 POSIX IPC（Inter-Process Communication）機制：
- **7-1 Message Queue**：kernel 維護一個訊息佇列，sender 用 `mq_send()` 丟訊息進去，receiver 用 `mq_receive()` 依序取出。Exercise 1：sender 反覆輸入字串直到 `exit`，receiver 反覆接收並印出，最後 close & unlink。
- **7-2 Shared Memory**：用 `shm_open()` 建一個共享記憶體物件，`ftruncate()` 設大小，`mmap()` 映射到自己的位址空間，兩個 process 直接讀寫同一塊實體記憶體。Exercise 2：producer 把使用者輸入寫進 shared memory，consumer 印出來。
- 兩者編譯都要加 **`-lrt`**（librt）。

---

# Part 1：Lab 7-1 Message Queue

## 2.1 核心觀念

### 什麼是 Message Queue
- 提供一種**簡單又有效率**的方式，在**兩個不相關的 process** 之間傳遞資料。
- Process A 把 message 放進 queue，Process B 從 queue 取出；訊息是一則一則的（有邊界），不是位元組串流。
- Linux 內部結構：`struct msqid_ds` 裡有 `ipc_perm` 結構（擁有者與權限）、`*msg_first`、`*msg_last` 指標，指向一串 `struct msg` 的 linked list，每個 `msg` 有 `*next`、`type`、`message`。

### POSIX Message Queue 五個主要函式
| 函式 | 作用 |
|---|---|
| `mq_open()` | 建立新的 queue 或開啟既有的 queue |
| `mq_close()` | 關閉 queue（**不會刪除 queue**） |
| `mq_unlink()` | 刪除 queue（移除名字） |
| `mq_send()` | 寫一則訊息進 queue |
| `mq_receive()` | 從 queue 讀一則訊息 |

### close vs unlink（必考）
- `mq_close(mqd)`：只是這個 process **不再使用**這個 descriptor，queue 還存在，訊息還在。
- `mq_unlink(name)`：**移除 queue 的名字**；之後沒有 `O_CREAT` 的 `mq_open` 就無法用這個名字開到它。真正**銷毀** queue 的時機：reference count 降到 0 時立即銷毀，否則**延後到所有參照都 close 之後**。
- 慣例：sender 只 close；最後一個用完的（receiver）負責 unlink。

### mq_attr 屬性
```c
struct mq_attr {
    long mq_flags;    /* 0 或 O_NONBLOCK            [mq_getattr(), mq_setattr()] */
    long mq_maxmsg;   /* queue 最多幾則訊息          [mq_open(), mq_getattr()] */
    long mq_msgsize;  /* 每則訊息最大 bytes          [mq_open(), mq_getattr()] */
    long mq_curmsgs;  /* 目前 queue 中有幾則訊息     [mq_getattr()] */
};
```
- **`mq_open()` 只需要指定兩個欄位：`mq_maxmsg` 和 `mq_msgsize`**（其他欄位建立時會被忽略）。
- 傳 `NULL` 則使用系統預設值（補充：Linux 預設通常是 maxmsg=10、msgsize=8192）。

### mq_receive 的 buffer 大小規則（必考）
`msg_len`（buffer 可用空間）**必須 ≥ queue 的 `mq_msgsize`**，否則 `mq_receive` 失敗（errno = EMSGSIZE）。

### mqd_t
`mq_open` 回傳的 message queue descriptor，型別 `mqd_t`，在 Linux 中就是 `int`。

## 2.2 API 速查表

| 函式 | 原型 | 回傳值 | 重點參數 | 範例 |
|---|---|---|---|---|
| `mq_open` | `mqd_t mq_open(const char *name, int oflag, ... /* mode_t mode, struct mq_attr *attr */);` | descriptor；失敗 `(mqd_t)-1` | `name` 以 `/` 開頭；oflag 見下；有 `O_CREAT` 時要給 mode、attr | `mq_open("/myMsgQueue", O_WRONLY\|O_CREAT, 0644, NULL)` |
| `mq_close` | `int mq_close(mqd_t mqdes);` | 0 / -1 | — | `mq_close(mqd)` |
| `mq_unlink` | `int mq_unlink(const char *name);` | 0 / -1 | — | `mq_unlink("/myMsgQueue")` |
| `mq_send` | `int mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned int msg_prio);` | 0 / -1 | `msg_len` = 訊息長度；`msg_prio` 本課給 0（忽略優先權） | `mq_send(mqd, buf, strlen(buf)+1, 0)` |
| `mq_receive` | `ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio);` | 收到的 bytes 數 / -1 | `msg_len` ≥ `mq_msgsize`；`msg_prio` 給 NULL | `mq_receive(mqd, buf, attr.mq_msgsize, NULL)` |
| `mq_getattr`（補充，學生有用） | `int mq_getattr(mqd_t, struct mq_attr *);` | 0 / -1 | 查實際的 msgsize | — |
| `fgets` | `char *fgets(char *s, int n, FILE *stream)` | 失敗/EOF 回 NULL | 從 stream 讀最多 n-1 字元，**會保留換行** | `fgets(buf, BUFSIZ, stdin)` |
| `strcmp` | `int strcmp(s1, s2)` | 相等回 0 | — | `strcmp(message, "hello")` |

標頭：`<fcntl.h>`（`O_*` 常數）、`<sys/stat.h>`（mode 常數）、`<mqueue.h>`。

**oflag：**
| Flag | 意義 |
|---|---|
| `O_CREAT` | 不存在就建立 |
| `O_EXCL` | 搭配 `O_CREAT`：若已存在就失敗（獨佔建立） |
| `O_RDONLY` / `O_WRONLY` / `O_RDWR` | 只讀／只寫／讀寫 |
| `O_NONBLOCK` | 非阻塞模式（queue 空時 receive 直接回錯，不等待） |

**mode（`<sys/stat.h>`）：**
| 常數 | 八進位 | 意義 |
|---|---|---|
| `S_IRUSR` `S_IWUSR` `S_IXUSR` | 0400 0200 0100 | User read/write/execute |
| `S_IRGRP` `S_IWGRP` `S_IXGRP` | 040 020 010 | Group read/write/execute |
| `S_IROTH` `S_IWOTH` `S_IXOTH` | 04 02 01 | Other read/write/execute |

例：`0644` = `S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH`。

## 2.3 程式範例逐行解說

### 投影片 example1/sender.c
```c
#include <fcntl.h>      // defines O_* constants for mq_open()
#include <mqueue.h>
#include <stdio.h>
#include <string.h>
#define NAME "/myMsgQueue"
#define MAX_MSGSIZE 1000

int main() {
    mqd_t mqd = mq_open(NAME, O_WRONLY | O_CREAT, 0644, NULL);   // 預設屬性
    if (mqd == -1) { perror("Failed to open or create the message queue. \n"); return 1; }
    printf("Send to receiver: \n");
    while (1) {
        char buffer[MAX_MSGSIZE];
        for (int i = 0; i < sizeof(buffer); i++) buffer[i] = '\0';   // 字串以 '\0' 結尾
        printf("> ");
        fgets(buffer, MAX_MSGSIZE, stdin);
        int send = mq_send(mqd, buffer, strlen(buffer) + 1, 0);     // +1 把 '\0' 一起送
        if (send == -1) { perror("Failed to send a message. \n"); return 1; }
    }
    return 0;
}
```
- `strlen(buffer) + 1`：把結尾 `'\0'` 一起送，receiver 才能直接當字串印。
- `fgets` 讀進來的字串**含換行 `\n`**，所以 receiver 用 `printf("received: %s", buffer)` 不必再加 `\n`。
- 這個範例是無窮迴圈，只能 Ctrl+C 結束，**永遠不會 close**。

### 投影片 example1/receiver.c
```c
mqd_t mqd = mq_open(NAME, O_RDONLY | O_CREAT, 0644, NULL);
...
while (1) {
    char buffer[MAX_MSGSIZE];                                   // 1000 bytes
    ssize_t msg_size = mq_receive(mqd, buffer, 8192, NULL);     // 告訴 kernel buffer 有 8192
    if (msg_size == -1) { perror("Failed to read the message. \n"); return 1; }
    printf("received: %s", buffer);
}
int close = mq_close(mqd);      // 0 -> success, -1 -> fail
...
int unlink = mq_unlink(NAME);
```
- 結果：sender 打 `test`、`123`、`456`，receiver 依序印 `received: test`、`received: 123`、`received: 456`。
- **問題點（考試可能問）**：
  1. 用預設屬性時 `mq_msgsize` 是 8192，所以 `msg_len` 必須給 ≥ 8192，範例給 8192 沒錯；**但實際 buffer 只有 1000 bytes**，若有人送超過 1000 bytes 的訊息會 buffer overflow。正確做法是 buffer 至少開 `mq_msgsize`（或用 `mq_getattr` 查完再 malloc——學生版就是這樣做）。
  2. `while(1)` 沒有出口，下面的 `mq_close`、`mq_unlink` **永遠執行不到**——這就是 Exercise 要加入 `exit` 機制的原因。
  3. 變數取名 `close`、`unlink` 會遮蔽同名的 system call 函式，風格不佳。

### 學生作業 practice/lab7/ex1/sender.c
```c
struct mq_attr attr;
attr.mq_flags = 0;  attr.mq_maxmsg = 10;  attr.mq_msgsize = MAX_MSGSIZE;  attr.mq_curmsgs = 0;
mqd_t mqd = mq_open(NAME, O_WRONLY | O_CREAT, 0644, &attr);   // 自訂屬性
if (mqd == (mqd_t)-1) { perror(...); return 1; }
printf("Send to receiver:\n");
while (1) {
    char buffer[MAX_MSGSIZE];
    memset(buffer, '\0', sizeof(buffer));
    printf("> "); fflush(stdout);
    if (fgets(buffer, MAX_MSGSIZE, stdin) == NULL) strcpy(buffer, "exit");   // Ctrl+D 也當 exit
    buffer[strcspn(buffer, "\n")] = '\0';                                     // 去掉換行
    if (mq_send(mqd, buffer, strlen(buffer) + 1, 0) == -1) { perror(...); return 1; }
    if (strcmp(buffer, "exit") == 0) break;      // exit 也送出去，通知 receiver 結束
}
if (mq_close(mqd) == -1) { ... }                 // sender 只 close，不 unlink
```
### practice/lab7/ex1/receiver.c
```c
mqd_t mqd = mq_open(NAME, O_RDONLY | O_CREAT, 0644, &attr);
mq_getattr(mqd, &attr);                      // 查實際 mq_msgsize
char *buffer = malloc(attr.mq_msgsize);      // buffer 大小 >= mq_msgsize
while (1) {
    ssize_t msg_size = mq_receive(mqd, buffer, attr.mq_msgsize, NULL);
    if (msg_size == -1) { ...; return 1; }
    if (strcmp(buffer, "exit") == 0) { printf("Exiting receive...\n"); break; }
    printf("received: %s\n", buffer);
}
free(buffer);
mq_close(mqd);
mq_unlink(NAME);                             // receiver 負責刪除 queue
```
互動結果（與投影片 Exercise 結果一致）：
```
$ ./sender                    $ ./receiver
Send to receiver:             received: test
> test                        received: 123
> 123                         Exiting receive...
> exit
```

**評論**：
- 寫得完整且比範例穩健：自訂屬性、`mq_getattr` + `malloc` 確保 buffer 夠大、`exit` 會先送給 receiver 再離開、EOF 也處理、sender 只 close、receiver close + unlink。符合題目全部要求。
- 小提醒：
  1. 因為用 `O_CREAT` 但沒加 `O_EXCL`，如果系統中已有**舊的同名 queue**（例如之前跑投影片範例時用預設 8192 建的、又沒被 unlink），`attr` 會被忽略、直接開舊的。receiver 用 `mq_getattr` 已防範這點；sender 送的訊息 ≤1000 bytes，也不會出錯。
  2. 若 receiver 沒在跑，sender 送超過 `mq_maxmsg`（10）則訊息後，`mq_send` 會**阻塞**等待空位。
  3. 去掉了換行，所以 receiver 的 printf 自己補 `\n`；和投影片範例（保留換行）寫法不同，但都正確。
  4. 若 receiver 因 Ctrl+C 中途結束，queue 沒被 unlink 會殘留（補充：可在 `/dev/mqueue/` 看到，`rm /dev/mqueue/myMsgQueue` 清掉）。
- `practice/lab7` 資料夾裡已有編好的 `sender`、`receiver`、`producer`、`consumer` 執行檔。

## 2.4 Exercise 1 題目（50 pts.）
- **sender.c**：建立 message queue；使用者可以反覆輸入字串，直到輸入 `"exit"`；把訊息送進 queue。
- **receiver.c**：反覆從 queue 收訊息並印在螢幕上；最後 close & unlink queue。
- 提示可用 `fgets(buf, BUFSIZ, stdin)`、`strcmp(message, "hello")`（相等回 0）。
- 參考解法：即上面學生版程式（已完整）。

---

# Part 2：Lab 7-2 Shared Memory

## 3.1 核心觀念

### 兩種讓不相關 process 共享記憶體的技術
| | Shared memory-mapped file | Shared memory（POSIX shm） |
|---|---|---|
| 需不需要檔案 | **需要明確建立一個檔案**來映射 | **不需要**建立對應的檔案 |
| 原理 | 兩個 process 的 page table 中，對應映射區的 PT entries 指向同一組實體記憶體分頁 | 同左，只是物件由 kernel 在 `/dev/shm` 建立 |

### 位置
每個 process 的位址空間（stack、heap、data、text）中，有一段 **shared memory (mapped)** 區域，P1 和 P2 的這段區域都對應到**同一塊實體 shared memory**。一方寫入，另一方**立刻**看得到，不需要經過 kernel 複製（所以是最快的 IPC，補充）。

### POSIX Shared Memory 使用流程
1. `shm_open()`：以名稱開啟或建立物件，回傳 **file descriptor**（就像 `open()`）。
2. `ftruncate()`：設定物件大小（新建立的物件大小是 0，一定要設）。
3. `mmap()`：把 fd 映射進自己的虛擬位址空間，**flags 要用 `MAP_SHARED`**。
4. 讀寫回傳的指標。
5. `munmap()`：解除映射；`close(fd)`。
6. `shm_unlink()`：刪除物件。

### `/dev/shm` 與 kernel persistence（必考）
- Linux 把 shared memory 物件建成**偽檔案（pseudo-file）**，放在特殊位置 **`/dev/shm`**。
- Shared memory 具有 **kernel persistence**：**即使目前沒有任何 process 開著它，物件依然存在**，直到系統關機或呼叫 `shm_unlink()`。
  → 所以 producer 結束後，consumer 還能讀到資料。

### 同步問題
和 memory-mapped file 一樣，若兩個以上 process **同時**存取，需要用 **semaphore**（或其他同步機制）同步（課程後面的 Lab 會介紹）。本 Lab 是「producer 先寫完、consumer 後讀」，靠人工控制執行順序避開競爭。

### Message Queue vs Shared Memory（補充比較）
| | Message Queue | Shared Memory |
|---|---|---|
| 資料怎麼傳 | 經由 kernel 複製（send 複製進 kernel、receive 複製出來） | 直接讀寫同一塊記憶體，不複製 |
| 速度 | 較慢 | 最快 |
| 同步 | kernel 處理（queue 空時 receive 阻塞） | **要自己同步**（semaphore 等） |
| 資料單位 | 一則一則的訊息 | 一塊位元組，自己定義格式 |
| 物件位置 | `/dev/mqueue` | `/dev/shm` |

## 3.2 API 速查表

| 函式 | 原型 | 回傳值 | 重點 | 範例 |
|---|---|---|---|---|
| `shm_open` | `int shm_open(const char *name, int oflag, mode_t mode);`（`<sys/mman.h>`） | fd；失敗 -1 | oflag：`O_CREAT` `O_EXCL` `O_RDONLY` `O_RDWR` `O_TRUNC`（**沒有 O_WRONLY**）；mode 權限 | `shm_open(NAME, O_CREAT\|O_EXCL, 0666)` |
| `ftruncate` | `int ftruncate(int fildes, off_t length);`（`<unistd.h>`） | 0 / -1 | 設定物件（檔案）大小 | `ftruncate(fd, 2048)` |
| `mmap` | `void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);` | 映射起始位址；失敗 **`MAP_FAILED`** | addr 給 NULL 讓 kernel 選；prot；flags；offset 從檔案哪裡開始 | `mmap(NULL, 1024, PROT_WRITE, MAP_SHARED, shm_fd, 0)` |
| `munmap` | `int munmap(void *addr, size_t length);` | 0 / -1 | 解除映射 | `munmap(address, 2048)` |
| `shm_unlink` | `int shm_unlink(const char *name);` | 0 / -1 | 刪除物件 | `shm_unlink(NAME)` |

**shm_open oflag（`<fcntl.h>`）**
| Flag | 意義 |
|---|---|
| `O_CREAT` | 不存在就建立 |
| `O_EXCL` | 搭配 O_CREAT 獨佔建立（已存在就失敗） |
| `O_RDONLY` | 唯讀 |
| `O_RDWR` | 讀寫 |
| `O_TRUNC` | 把物件截成長度 0 |

**mmap 的 prot**
| 值 | 意義 |
|---|---|
| `PROT_READ` | 可讀 |
| `PROT_WRITE` | 可寫 |
| `PROT_EXEC` | 可執行 |
| `PROT_NONE` | 不可存取 |

**mmap 的 flags（本課重點兩個）**
| 值 | 意義 |
|---|---|
| **`MAP_SHARED`** | 對映射區的修改**其他 process 看得到**，並寫回底層檔案 |
| **`MAP_PRIVATE`** | 修改是**私有的**（copy-on-write），別人看不到 |
| 其他 | `MAP_ANONYMOUS`（不對應檔案）、`MAP_FIXED`（精確使用 addr）、`MAP_LOCKED`、`MAP_HUGETLB`、`MAP_NORESERVE`、`MAP_POPULATE`、`MAP_UNINITIALIZED` |

**編譯：** POSIX IPC 程式要連結 **librt**：`gcc -o producer producer.c -lrt`（補充：glibc 2.34 以後這些函式已併入 libc，不加 `-lrt` 通常也能編，但照投影片加上最保險）。

## 3.3 程式範例逐行解說

### 投影片 producer.c
```c
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>       // ftruncate() & close()
#define NAME "/myShareMemory"
#define SMSIZE 1024
int main() {
    int fd = shm_open(NAME, O_CREAT | O_RDWR, 0666);          // file descriptor
    if (fd == -1) { perror("Failed to open or create the shared memory. \n"); return 1; }
    if (ftruncate(fd, SMSIZE) == -1) { perror("Failed to resize. \n"); return 1; }
    void *map_addr = mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map_addr == MAP_FAILED) { perror("Failed to create the map. \n"); return 1; }
    char *ptr, buffer[SMSIZE];
    ptr = map_addr;
    sprintf(ptr, "%s", buffer);                                // 把 buffer 寫進 shared memory
    if (munmap(map_addr, 1024) == -1) { ...; return 1; }
    if (close(fd) == -1) { ...; return 1; }
    return 0;
}
```
- 注意：範例中 `buffer` **沒有初始化**就被 `sprintf` 寫進 shared memory，內容是未定義的垃圾——這只是骨架，Exercise 要你改成讀使用者輸入。
- 結束時 **沒有 shm_unlink**——故意的，因為 kernel persistence 讓資料留給 consumer。
- 錯誤檢查要和 **`MAP_FAILED`** 比，不是和 NULL 比。

### 投影片 consumer.c
```c
int fd = shm_open(NAME, O_RDONLY, 0666);
void *map_addr = mmap(NULL, 1024, PROT_READ, MAP_SHARED, fd, 0);
printf("%p\n", map_addr);      // 只印出映射的「位址」，不是內容
return 0;
```
- 開啟方式要對應：`O_RDONLY` 開的 fd，mmap 只能用 `PROT_READ`（若要 `PROT_WRITE` 會失敗，EACCES）。
- `%p` 印的是**位址**；Exercise 要求印**內容**，要改成 `printf("%s\n", (char *)map_addr)`。
- 範例沒有 munmap、close、shm_unlink。

### 學生作業 practice/lab7/ex2/producer.c
```c
int fd = shm_open(NAME, O_CREAT | O_RDWR, 0666);         // 建立或開啟
ftruncate(fd, SMSIZE);                                   // 設 1024 bytes
char *ptr = mmap(NULL, SMSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
printf("Enter the message: "); fflush(stdout);
if (fgets(ptr, SMSIZE, stdin) == NULL) ptr[0] = '\0';   // 直接讀進 shared memory
ptr[strcspn(ptr, "\n")] = '\0';                          // 去掉換行
munmap(ptr, SMSIZE);
close(fd);                                               // 不 unlink，資料留給 consumer
```
### practice/lab7/ex2/consumer.c
```c
int fd = shm_open(NAME, O_RDONLY, 0666);                 // 開已存在的
char *ptr = mmap(NULL, SMSIZE, PROT_READ, MAP_SHARED, fd, 0);
printf("Read from shared memory: %s\n", ptr);            // 印內容
munmap(ptr, SMSIZE);
close(fd);
shm_unlink(NAME);                                        // 讀完刪除物件
```
結果（與投影片一致）：
```
$ ./producer
Enter the message: hello
$ ./consumer
Read from shared memory: hello
```

**評論**：
- 正確且完整，每一步都有錯誤檢查，且改正了範例的兩個問題（未初始化 buffer、印位址）。
- 注意執行順序：**一定要先跑 producer 再跑 consumer**。consumer 會 `shm_unlink`，所以連續跑兩次 consumer，第二次會出現 `Failed to open the shared memory: No such file or directory`。
- 若使用者輸入剛好 1023 個字元以上，`fgets` 會截斷，仍有 `'\0'` 結尾，安全。
- 沒有同步機制：若 consumer 在 producer 寫入過程中讀，可能讀到不完整資料（本題不要求）。
- 執行中可用 `ls -l /dev/shm` 看到 `myShareMemory`（大小 1024）。

## 3.4 Exercise 2 題目（50 pts.）
- **producer.c**：建立 shared memory；使用者可以輸入訊息寫進 shared memory。
- **consumer.c**：把 shared memory 的內容印到螢幕上。
- 編譯：`gcc -o producer producer.c -lrt`、`gcc -o consumer consumer.c -lrt`。
- 參考解法：即上面學生版程式（已完整）。

---

## 4. 上機考實作步驟

### Exercise 1：Message Queue
```bash
mkdir -p ~/lab7/exercise1 && cd ~/lab7/exercise1
vim sender.c
vim receiver.c
gcc sender.c -o sender -lrt
gcc receiver.c -o receiver -lrt

# 開兩個 terminal（Ctrl+Shift+T 開新分頁）
./receiver          # terminal 1（也可以先開 sender，兩邊都有 O_CREAT）
./sender            # terminal 2：輸入 test、123、exit

# 除錯／清理殘留的 queue（補充）
ls /dev/mqueue/
cat /dev/mqueue/myMsgQueue      # 看 QSIZE 等資訊
rm /dev/mqueue/myMsgQueue
```

### Exercise 2：Shared Memory
```bash
mkdir -p ~/lab7/exercise2 && cd ~/lab7/exercise2
vim producer.c
vim consumer.c
gcc -o producer producer.c -lrt
gcc -o consumer consumer.c -lrt
./producer          # Enter the message: hello
ls -l /dev/shm      # 看到 myShareMemory（kernel persistence）
./consumer          # Read from shared memory: hello
ls -l /dev/shm      # 已被 shm_unlink 刪除
```

### 考場最小骨架（背起來）
```c
/* message queue */
mqd_t q = mq_open("/q", O_CREAT | O_RDWR, 0644, NULL);
mq_send(q, msg, strlen(msg) + 1, 0);
struct mq_attr a; mq_getattr(q, &a);
char *buf = malloc(a.mq_msgsize);
mq_receive(q, buf, a.mq_msgsize, NULL);
mq_close(q); mq_unlink("/q");

/* shared memory */
int fd = shm_open("/s", O_CREAT | O_RDWR, 0666);
ftruncate(fd, 1024);
char *p = mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
strcpy(p, "hello");
munmap(p, 1024); close(fd); /* 讀方最後 */ shm_unlink("/s");
```

---

## 5. 常見錯誤與陷阱

**Message Queue**
1. 名稱沒有以 `/` 開頭（如 `"myMsgQueue"`）→ `mq_open` 失敗（EINVAL）。
2. `mq_receive` 的 `msg_len` **小於** `mq_msgsize` → EMSGSIZE 錯誤；`msg_len` 謊報比實際 buffer 大 → 有 overflow 風險。
3. 送字串時用 `strlen(buf)` 沒 `+1` → receiver 收到的沒有 `'\0'`，印出垃圾（除非 buffer 事先清 0）。
4. 以為 `mq_close` 會刪 queue → 不會，要 `mq_unlink`。
5. 沒 unlink 導致舊 queue 殘留，新程式的 `attr` 被忽略。
6. `fgets` 會把 `\n` 一起讀進來，用 `strcmp(buf, "exit")` 比較會失敗——要先去掉換行，或比 `"exit\n"`。
7. 判斷 `mq_open` 失敗要和 `(mqd_t)-1` 比。
8. 忘了 `-lrt`（舊 glibc 會 undefined reference to `mq_open`）。
9. 非 root 時 `mq_maxmsg` 不能超過系統上限（`/proc/sys/fs/mqueue/msg_max`，預設 10），設 11 以上會 EINVAL（補充）。

**Shared Memory**
1. 建立後**忘了 `ftruncate`** → 物件大小為 0，存取映射區會 **SIGBUS**。
2. 判斷 `mmap` 失敗要用 `MAP_FAILED`，不是 `NULL`。
3. 用 `MAP_PRIVATE` → 寫入的資料別的 process 看不到。
4. `shm_open` 用 `O_RDONLY` 卻 `mmap` 要 `PROT_WRITE` → 權限錯誤。
5. 誤用 `O_WRONLY`（shm_open 只有 `O_RDONLY` / `O_RDWR`）。
6. producer 結束時就 `shm_unlink` → consumer 打不開。
7. consumer 用 `%p` 印出的是位址不是內容。
8. 多個 process 同時讀寫沒有同步 → race condition（需要 semaphore）。
9. 忘了 `-lrt`。

---

## 6. 筆試模擬題

**Q1（選擇）** 下列哪個函式會「刪除」POSIX message queue？
(A) `mq_close`　(B) `mq_unlink`　(C) `mq_receive`　(D) `mq_open` 加 `O_TRUNC`
> **答：B**。`mq_close` 只關閉 descriptor，不刪 queue。

**Q2（是非）** 呼叫 `mq_unlink()` 時，若還有其他 process 開著這個 queue，queue 會立即被銷毀、其他 process 讀取會失敗。
> **答：錯**。名字立即移除，但銷毀**延後到所有參照都 close 之後**。

**Q3（選擇）** 建立 message queue 時，`mq_open()` 會用到 `struct mq_attr` 的哪兩個欄位？
(A) mq_flags、mq_curmsgs　(B) mq_maxmsg、mq_msgsize　(C) mq_flags、mq_maxmsg　(D) mq_msgsize、mq_curmsgs
> **答：B**。

**Q4（簡答）** `mq_receive()` 的 `msg_len` 有什麼限制？
> **答：** 必須大於或等於 queue 的 `mq_msgsize`，否則呼叫失敗（EMSGSIZE）；而 buffer 實際大小也必須至少 `msg_len`。

**Q5（輸出預測）** sender 呼叫 `mq_send(mqd, "hi", 2, 0)`（沒有 +1），receiver 用一個**未清空**的 buffer 接收後 `printf("%s", buf)`，可能發生？
> **答：** 收到的 2 bytes 沒有 `'\0'`，printf 會繼續印出 buffer 後面的殘留垃圾，直到遇到某個 `'\0'`。應送 `strlen+1`。

**Q6（選擇）** Linux 中 POSIX shared memory 物件存放在哪裡？
(A) `/tmp`　(B) `/proc/shm`　(C) `/dev/shm`　(D) `/var/shm`
> **答：C**。

**Q7（簡答）** 何謂 kernel persistence？對 producer/consumer 有什麼意義？
> **答：** 物件在沒有任何 process 開著時仍然存在，直到系統關機或呼叫 `shm_unlink()`。因此 producer 寫完資料結束後，consumer 之後再執行仍能讀到資料。

**Q8（排序）** 將 producer 使用 shared memory 的步驟排序：① `mmap` ② `shm_open` ③ 寫入資料 ④ `ftruncate` ⑤ `munmap` + `close`
> **答：** ② → ④ → ① → ③ → ⑤。

**Q9（選擇）** 要讓修改對其他 process 可見，`mmap` 的 flags 要用？
(A) `MAP_PRIVATE`　(B) `MAP_ANONYMOUS`　(C) `MAP_SHARED`　(D) `MAP_FIXED`
> **答：C**。`MAP_PRIVATE` 的修改是私有的。

**Q10（是非）** `mmap()` 失敗時回傳 `NULL`。
> **答：錯**。回傳 `MAP_FAILED`（即 `(void *)-1`）。

**Q11（簡答）** shared memory-mapped file 與 POSIX shared memory 最主要的差別？
> **答：** 前者需要明確建立一個實體檔案來映射；後者不需要建立對應的檔案（kernel 在 /dev/shm 建立偽檔案）。

**Q12（簡答）** 為什麼兩個 process 同時存取 shared memory 時需要 semaphore，而 message queue 通常不用？
> **答：** shared memory 只是共用一塊記憶體，kernel 不管讀寫順序，同時存取會產生 race condition，必須自己同步；message queue 的 send/receive 由 kernel 管理，queue 空時 receive 會阻塞、訊息一則一則完整傳遞，本身就有同步效果。

**Q13（選擇）** 編譯使用 POSIX IPC 的程式時，投影片要求加上哪個 gcc 選項？
(A) `-lpthread`　(B) `-lm`　(C) `-lrt`　(D) `-shared`
> **答：C**。連結 librt。

**Q14（輸出預測）** 投影片 consumer.c 執行後印出的是什麼？
> **答：** 映射區的**位址**（`%p`，例如 `0x7f3a1c2d4000`），而不是 shared memory 的內容。

**Q15（程式填空）** receiver 在收到 `"exit"` 時跳出迴圈：
```c
if (__________(buffer, "exit") == 0) break;
```
> **答：** `strcmp`。
