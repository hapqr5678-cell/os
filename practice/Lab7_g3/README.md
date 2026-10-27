# Lab7_g3

> 來源:review/predict/Lab7_predict.md

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
