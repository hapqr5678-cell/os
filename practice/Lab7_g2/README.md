# Lab7_g2

> 來源:review/predict/Lab7_predict.md

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
