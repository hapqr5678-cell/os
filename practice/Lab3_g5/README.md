# Lab3_g5

> 來源:review/predict/Lab3_predict.md

## 第 5 題：write() vs fwrite() 一次寫 1 byte，比較時間並用 strace 觀察

### 題目
仿照 example4，但改成「**寫**」：分別用 `write()` 和 `fwrite()` 一次寫 1 byte、共寫 1MB（1048576 次）到檔案，印出花費秒數；再用 `strace -c` 比較 `write` system call 的次數，並解釋原因。

**預期結果**
```
$ ./q5_write
0.463326
$ ./q5_fwrite
0.013798
$ strace -c ./q5_write    → write 呼叫 1048577 次
$ strace -c ./q5_fwrite   → write 呼叫 257 次
```

### 變形自哪裡／考什麼
- **example4（p.29~31）** read() vs fread() 的鏡像版。
- **p.9 strace**、**p.24 fwrite**、**p.25 fclose（會 flush buffer）**。
- 觀念：library 在 user space 有 buffer（4096 bytes），滿了才發一次 `write()` → syscall 次數 ≈ 1048576 / 4096 = 256。每次 syscall 都要 user mode ↔ kernel mode 切換，所以 `write()` 一次 1 byte 很慢。這就是 Exercise 的學習目標。

### 可能性：中
example4 是整份簡報的「結論實驗」，但考場上跑 10MB＋strace 很花時間，比較可能縮小規模或改成「寫」。也可能直接考原版 read vs fread（解法結構相同，把 write 換 read、`"w"` 換 `"r"` 即可）。

### 參考答案（q5_write.c / q5_fwrite.c）
```c
// Q5-a：用 write() 一次寫 1 byte，共寫 1MB，量時間
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#define TOTAL (1024 * 1024)

int main() {
    struct timespec t1, t2;
    char c = 'A';
    int fd = open("out_write.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd == -1) { perror("open"); return 1; }

    clock_gettime(CLOCK_MONOTONIC, &t1);
    for (int i = 0; i < TOTAL; i++)
        write(fd, &c, 1);               // 每次都是一個 system call
    clock_gettime(CLOCK_MONOTONIC, &t2);
    close(fd);

    printf("%f\n", (t2.tv_sec - t1.tv_sec) + (t2.tv_nsec - t1.tv_nsec) / 1e9);
    return 0;
}
```
```c
// Q5-b：用 fwrite() 一次寫 1 byte，共寫 1MB，量時間
#include <stdio.h>
#include <time.h>

#define TOTAL (1024 * 1024)

int main() {
    struct timespec t1, t2;
    char c = 'A';
    FILE *fp = fopen("out_fwrite.txt", "w");
    if (fp == NULL) { perror("fopen"); return 1; }

    clock_gettime(CLOCK_MONOTONIC, &t1);
    for (int i = 0; i < TOTAL; i++)
        fwrite(&c, 1, 1, fp);           // 先放進 user-space buffer，滿了才 write()
    fclose(fp);                         // fclose 會 flush 剩下的資料
    clock_gettime(CLOCK_MONOTONIC, &t2);

    printf("%f\n", (t2.tv_sec - t1.tv_sec) + (t2.tv_nsec - t1.tv_nsec) / 1e9);
    return 0;
}
```
> 計時用 `clock_gettime`：簡報 p.30 只秀出執行結果（秒數），沒附程式碼；若考題要計時，用這個或直接 `time ./prog` 都可以。題目重點在 strace 的 calls 數。

### ✅ 已驗證
```
$ gcc -Wall q5_write.c -o q5_write; gcc -Wall q5_fwrite.c -o q5_fwrite
$ ./q5_write
0.463326
$ ./q5_fwrite
0.013798
$ ls -l out_write.txt out_fwrite.txt
-rw-r--r-- 1 root root 1048576 Oct 27 12:01 out_fwrite.txt
-rw-r--r-- 1 root root 1048576 Oct 27 12:01 out_write.txt
$ strace -c ./q5_write 2>&1 | grep write
100.00   11.708793          11   1048577           write
$ strace -c ./q5_fwrite 2>&1 | grep write
100.00    0.002124           8       257           write
```
（多出的那 1 次 write 是最後 `printf` 印秒數。）

### 容易踩的坑
1. **忘了 `fclose`／沒 flush**：fwrite 最後一塊留在 buffer，計時沒算到、檔案也可能少資料。計時要包含 `fclose`。
2. `strace -c` 的統計輸出在 **stderr**，要 `2>&1` 才能接 `grep`。
3. 開 `-O2` 不影響 syscall 次數，但為了公平兩支都用同樣選項編譯。
4. 讀檔版（原 example4）要先 `sudo sh -c "echo 3 > /proc/sys/vm/drop_caches"` 清 page cache 才公平（p.30）；考場若沒 sudo 就說明即可。
5. 解釋時說「fread/fwrite **本質上還是呼叫 read/write**，只是一次搬 4096 bytes」，不要說成「不用 system call」。
