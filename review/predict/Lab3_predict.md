# Lab 3 上機考猜題：System Call vs APIs

> 依據：`Lab3.pptx`（33 頁，含 example1~4 的程式截圖）＋ `practice/Lab3_ex/ex.c`
> 範圍原則：**只出簡報教過的東西**。所有參考答案都在 gcc 13.3（Ubuntu 24.04）實際編譯、執行過。

---

## 0. 這個 Lab 專屬的出題邏輯

| # | 觀察 | 對出題的意義 |
|---|---|---|
| 1 | Exercise（p.32）的重點是**限制**：「只用 `<unistd.h>`」，助教還會**看原始碼**抓有沒有偷 include `<stdio.h>` | 考題最可能保留這個限制，只換「要印/讀的東西」。評分會看 include 那幾行 |
| 2 | 學習目標寫的是「了解為何偏好包裝後的函式」 | 題目會讓你**親手做 printf/scanf 幫你做的事**（數字↔字元轉換、算字串長度、處理負號），體會 library 的方便 |
| 3 | 簡報分兩條線：system call I/O（p.13~21）與 C library I/O（p.22~28），最後 example4（p.29~31）比較兩者 | 變形題會在「同一件事用兩套 API 各寫一次」或「用 strace 證明 syscall 次數差」之間擇一 |
| 4 | example2（p.20~21）特別講 **fd 0/1/2**，而且示範 `write(1, ...)` | 「印到螢幕」= `write(1,...)`；反過來「從鍵盤讀」= `read(0,...)`，這是最自然的延伸（簡報講了 fd 0 但沒有範例 → 很適合拿來考） |
| 5 | example1（p.17~19）是 open → write → close、再 open → read → close | 檔案相關題目一定是這四個 syscall 的組合，flags 用 `O_CREAT \| O_WRONLY \| O_TRUNC`、mode 用 `0644` 這種八進位 |
| 6 | 權限碼（p.10~12）有 `ls -l`、八進位 755/0644、`chmod 0244` 後 `cat` 被拒 | 可能要求「用 open 建立指定權限的檔案，並用 `ls -l` 給助教看」 |
| 7 | 簡報沒教：`lseek`、`dup/dup2`、`stat`、`snprintf`、`atoi`、`string.h`、`errno` 的處理細節 | 不會考；**而且在只能用 unistd.h 的題目裡也不能用**（`atoi`、`strlen` 都不在 unistd.h） |

**一句話總結：**Lab 3 的上機考 = 「在 *只准用 system call* 的限制下，自己把 printf/scanf/cp 做出來」，或「用 strace 證明 library 有 buffer、syscall 比較少」。

---

## 猜題總覽（依可能性排序）

| # | 題目 | 可能性 | 主要變形自 |
|---|---|---|---|
| 1 | 只用 unistd.h 印出字串＋整數陣列（含負數） | 高 | Exercise p.32 |
| 2 | 只用 unistd.h 從 stdin（fd 0）讀兩個整數並印出和 | 高 | Exercise p.32 ＋ fd 0（p.20） |
| 3 | 只用 system call 複製檔案（mycp），並印出複製了幾 bytes | 中高 | example1 p.17~19 ＋ Exercise |
| 4 | 用 open 指定權限 0640 建檔，把整數寫進檔案，`ls -l` 驗證 | 中 | p.10~13 權限 ＋ Exercise |
| 5 | write() vs fwrite() 一次寫 1 byte，比較時間與 strace -c | 中 | example4 p.29~31 |
| 6 | 用 fopen/fread/fwrite/fclose 改寫檔案複製 | 低 | example3 p.26~28 |

---

## 第 1 題：只用 `<unistd.h>` 印出字串與整數陣列（含負數）

### 題目
只能 `#include <unistd.h>`（不可用 `<stdio.h>`、`<string.h>`），實作：
- `my_puts(const char *s)`：印出字串（不自動換行）
- `my_print_int(int a)`：印出整數（不換行）

用它們印出陣列 `{3, -15, 0, 2147483647, -2147483648}`（以空白分隔），以及元素個數。

**預期輸出**
```
arr = 3 -15 0 2147483647 -2147483648
n = 5
```

### 變形自哪裡／考什麼
- 直接變形自 **Exercise（p.32）**：測資一模一樣含 `0`、`2147483647`、`-2147483648`。
- 多了「印字串」→ 必須自己寫 `strlen`（`string.h` 不能用），呼應 **example2（p.21）的 `write(1, buffer, ...)`**。
- 觀念：`write()` 只認 bytes，不認「整數」；數字要自己轉成字元 → 這就是為什麼偏好 `printf`。

### 可能性：高
Exercise 本身只要求「印 integer」，最省力的變形就是「多印一點東西、換一下格式」（同一行、空白分隔、前面加字串）。測資中的 `-2147483648` 幾乎一定會保留。

### 參考答案（q1.c）
```c
// Q1：只用 <unistd.h> 印出字串與整數陣列（含負數）
#include <unistd.h>

int my_strlen(const char *s) {
    int n = 0;
    while (s[n] != '\0') n++;
    return n;
}

void my_puts(const char *s) {           // 印字串（不自動換行）
    write(1, s, my_strlen(s));
}

void my_print_int(int a) {              // 印整數（不換行）
    char buf[12];                       // '-' + 10 位數 + 1 備用
    int i = 11;
    unsigned int u = (a < 0) ? -(unsigned int)a : (unsigned int)a;
    do {
        buf[i--] = '0' + u % 10;
        u /= 10;
    } while (u > 0);
    if (a < 0) buf[i--] = '-';
    write(1, buf + i + 1, 11 - i);
}

int main() {
    int arr[] = {3, -15, 0, 2147483647, -2147483648};
    int n = sizeof(arr) / sizeof(arr[0]);

    my_puts("arr = ");
    for (int k = 0; k < n; k++) {
        my_print_int(arr[k]);
        my_puts(k == n - 1 ? "\n" : " ");
    }
    my_puts("n = ");
    my_print_int(n);
    my_puts("\n");
    return 0;
}
```

### ✅ 已驗證
```
$ gcc -Wall -Wextra -Werror=implicit-function-declaration q1.c -o q1   # 只有 unistd.h，零警告
$ ./q1
arr = 3 -15 0 2147483647 -2147483648
n = 5
$ nm -u q1 | grep -E 'printf|puts|fwrite'    # 確認沒有連到 stdio 函式
（無輸出）
```

### 容易踩的坑
1. **`-2147483648` 取負會溢位**：`a = -a` 在 int 範圍外是 UB。要轉成 `unsigned int` 再取負：`-(unsigned int)a`。
2. **`0` 印不出來**：用 `while (u > 0)` 的話 0 一圈都不跑；要用 `do { } while`。
3. **buffer 太小**：`-2147483648` 是 11 個字元，`char buf[11]` 剛好不夠放結尾換行；開 12 以上。
4. 數字是倒著產生的（個位先出來），要從陣列尾端往前填，或最後反轉。
5. `write(1, s, sizeof(s))`：`s` 是指標時 `sizeof` 是 8，不是字串長度！要自己算長度。
6. 偷寫 `#include <stdio.h>` 或 `<string.h>` → 助教看程式碼直接扣分。

---

## 第 2 題：只用 `<unistd.h>` 從 stdin（fd 0）讀入兩個整數，輸出和

### 題目
只能 `#include <unistd.h>`。用 `read(0, ...)` 從鍵盤讀入兩個整數（可能是負數，中間用空白或換行分隔），輸出兩數相加結果。讀取失敗時把錯誤訊息寫到 **stderr（fd 2）**。

**輸入／輸出範例**
```
$ ./q2
Input two integers: 3 -5
sum = -2

$ printf '  -2147483648\n -1\n' | ./q2
Input two integers: sum = -2147483649
```

### 變形自哪裡／考什麼
- Exercise（p.32）的反方向：**自己做 scanf**（字元 → 數字）。
- **p.20**：「每個程式啟動 OS 自動開好 fd 0/1/2」。簡報只示範了 fd 1，fd 0 和 fd 2 正好留給考題。
- **p.14** read() 的回傳值：`>0` 讀到幾個、`0` = EOF、`-1` = 錯誤。

### 可能性：高
同時用到 read + write + fd 0/1/2，把 system call I/O 那一節完整串起來，又保留「只用 unistd.h」的限制，非常符合「exercise 換個小題目再用一次」的模式。

### 參考答案（q2.c）
```c
// Q2：只用 <unistd.h> 從 stdin(fd 0) 讀入兩個整數，輸出相加結果
#include <unistd.h>

int my_read_int(int *out) {             // 成功回傳 1，EOF 回傳 0
    char c;
    int sign = 1, got = 0;
    long long val = 0;                  // 用 long long 累加，-2147483648 才不會溢位
    // 1. 跳過空白、換行
    while (1) {
        if (read(0, &c, 1) <= 0) return 0;          // EOF
        if (c != ' ' && c != '\n' && c != '\t' && c != '\r') break;
    }
    // 2. 處理負號
    if (c == '-') {
        sign = -1;
        if (read(0, &c, 1) <= 0) return 0;
    }
    // 3. 逐字元累加數字
    while (c >= '0' && c <= '9') {
        val = val * 10 + (c - '0');
        got = 1;
        if (read(0, &c, 1) <= 0) break;
    }
    if (!got) return 0;
    *out = (int)(sign * val);
    return 1;
}

void my_print_int(long long a) {
    char buf[21];
    int i = 20;
    unsigned long long u = (a < 0) ? -(unsigned long long)a : (unsigned long long)a;
    do {
        buf[i--] = '0' + u % 10;
        u /= 10;
    } while (u > 0);
    if (a < 0) buf[i--] = '-';
    write(1, buf + i + 1, 20 - i);
}

int main() {
    int a, b;
    write(1, "Input two integers: ", 20);
    if (!my_read_int(&a) || !my_read_int(&b)) {
        write(2, "input error\n", 12);           // 錯誤訊息寫到 fd 2 (stderr)
        return 1;
    }
    write(1, "sum = ", 6);
    my_print_int((long long)a + b);             // 轉 long long 避免溢位
    write(1, "\n", 1);
    return 0;
}
```

### ✅ 已驗證
```
$ gcc -Wall -Wextra -Werror=implicit-function-declaration q2.c -o q2
$ printf '3 -5\n' | ./q2
Input two integers: sum = -2
$ printf '  -2147483648\n -1\n' | ./q2
Input two integers: sum = -2147483649
$ printf '100\n200\n' | ./q2
Input two integers: sum = 300
$ printf '7' | ./q2          # 只給一個數 → 寫到 stderr，回傳 1
Input two integers: input error
$ printf '12345 678\n' | strace -c -e trace=read ./q2   # 順便觀察：一次讀 1 byte，每個字元一次 syscall
100.00    0.004906         446        11           read     ← 10 個字元 + 1 次載入 libc
```

### 容易踩的坑
1. `read()` **不會自動補 `'\0'`**，也不會幫你轉數字；一個字元 `c` 要 `c - '0'` 才是數值。
2. 沒判斷 `read()` 回傳 `0`（EOF）→ 用 pipe 餵資料時會無窮迴圈。
3. 累加用 `int`：讀 `2147483648`（-2147483648 的絕對值）時溢位，要用 `long long` 累加。
4. 兩數相加也可能溢位（`2147483647 + 1`），輸出用 `long long`。
5. 提示字串用 `write(1, "...", 長度)` 時長度算錯（多算會印出亂碼 / `\0`）。
6. 加分說法：這題一次 `read` 1 byte，`strace` 會看到一個字元一次 syscall → 正好就是 example4 說 `scanf/fread` 比較快的原因。

---

## 第 3 題：只用 system call 複製檔案（自製 cp）

### 題目
寫 `mycp`，用法 `./mycp <src> <dst>`。只能用 `open/read/write/close`（`<unistd.h>`、`<fcntl.h>`），不能用 `<stdio.h>`。
- 目的檔不存在就建立，權限 **0644**；已存在則先清空。
- 複製完印出 `copied N bytes`。
- 來源檔開不起來要印錯誤訊息到 stderr 並 `return 1`。

**輸入／輸出範例**
```
$ printf 'Hello World\nline2\n' > src.txt
$ ./mycp src.txt dst.txt
copied 18 bytes
$ cat dst.txt
Hello World
line2
$ ls -l dst.txt
-rw-r--r-- 1 user user 18 ... dst.txt
```

### 變形自哪裡／考什麼
- **example1（p.17~19）**：write_file.c（`O_CREAT | O_WRONLY`, 0644）＋ read_file.c（`O_RDONLY`）合在同一支程式。
- **p.13** open 的 flags 與 mode；**p.14** read 回傳 0 = EOF；**p.15** write 寫回傳的 n 個。
- 輸出 bytes 數 → 又要用到 Exercise 的「自己印整數」。

### 可能性：中高
把 open/read/write/close 四頁全部用到、又能接上 Exercise，是「把所有小節串在一起」的典型題型。

### 參考答案（q3.c）
```c
// Q3：只用 system call (open/read/write/close) 複製檔案
// 用法：./mycp <src> <dst>
#include <unistd.h>
#include <fcntl.h>      // O_RDONLY, O_CREAT ... 這些 flag 定義在這裡

void my_print_int(long long a) {
    char buf[21];
    int i = 20;
    unsigned long long u = (a < 0) ? -(unsigned long long)a : (unsigned long long)a;
    do { buf[i--] = '0' + u % 10; u /= 10; } while (u > 0);
    if (a < 0) buf[i--] = '-';
    write(1, buf + i + 1, 20 - i);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        write(2, "usage: ./mycp <src> <dst>\n", 26);
        return 1;
    }
    int src = open(argv[1], O_RDONLY);
    if (src == -1) {
        write(2, "open src failed\n", 16);
        return 1;
    }
    int dst = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (dst == -1) {
        write(2, "open dst failed\n", 16);
        close(src);
        return 1;
    }

    char buffer[4096];
    ssize_t n;
    long long total = 0;
    while ((n = read(src, buffer, sizeof(buffer))) > 0) {   // 0 = EOF
        write(dst, buffer, n);           // 寫 n 個，不是 sizeof(buffer)！
        total += n;
    }

    close(src);
    close(dst);
    write(1, "copied ", 7);
    my_print_int(total);
    write(1, " bytes\n", 7);
    return 0;
}
```

### ✅ 已驗證
```
$ gcc -Wall -Wextra -Werror=implicit-function-declaration q3.c -o mycp
$ ./mycp src.txt dst.txt
copied 18 bytes
$ cat dst.txt
Hello World
line2
$ ls -l dst.txt
-rw-r--r-- 1 root root 18 Oct 27 12:01 dst.txt
$ ./mycp big.bin big2.bin && cmp big.bin big2.bin && echo SAME   # 10000 bytes 隨機二進位檔
copied 10000 bytes
SAME
$ ./mycp nofile x; echo rc=$?
open src failed
rc=1
$ echo "Hi" > src2.txt; ./mycp src2.txt dst.txt; cat dst.txt   # 覆蓋較短內容，O_TRUNC 有效
copied 3 bytes
Hi
```

### 容易踩的坑
1. **`write(dst, buffer, sizeof(buffer))`**：最後一塊不滿 4096 時會把垃圾一起寫進去。要寫 `n`。
2. **忘了 `O_TRUNC`**：dst 原本比較長時，尾巴會殘留舊內容（example1 的 write_file.c 就沒加，投影片剛好沒踩到）。
3. 有 `O_CREAT` 卻沒給第三個參數 mode → 權限變成隨機值。
4. mode 寫 `644`（十進位）而不是 `0644`（八進位）→ 權限完全錯。
5. `O_CREAT` 這些 flag 在 **`<fcntl.h>`**，不在 unistd.h；簡報 p.13 也是兩個一起 include。
6. 迴圈條件寫 `!= 0` 而不是 `> 0`：read 回傳 -1 時會無窮迴圈。

---

## 第 4 題：用 open() 指定權限建立檔案，把整數寫進檔案

### 題目
只用 `<unistd.h>`、`<fcntl.h>`：建立 `score.txt`，權限必須是 **`rw-r-----`（owner 讀寫、group 只讀、other 無）**，把 1~5 的平方一行一個寫進檔案。完成後螢幕印 `done`，並用 `ls -l`、`cat` 給助教看。

**輸入／輸出範例**
```
$ ./q4
done
$ ls -l score.txt
-rw-r----- 1 user user 12 ... score.txt
$ cat score.txt
1
4
9
16
25
```
（延伸口試題：`chmod 0244 score.txt` 之後 `cat` 會怎樣？→ `Permission denied`，因為 owner 只剩 w，p.12 示範過。）

### 變形自哪裡／考什麼
- **p.10~12 權限碼**：`rw-r-----` → 4+2 / 4 / 0 → **0640**；要會「權限字串 ↔ 八進位」互轉。
- **p.13** open 的第三個參數 mode。
- **Exercise** 的 my_print 改成「可以指定 fd」：印到螢幕是 fd 1，印到檔案是 open 回傳的 fd → 體會「一切都是檔案」（p.3）。

### 可能性：中
權限碼在簡報占了 3 頁，但單獨考 `chmod` 太簡單，最可能是和 open 的 mode 結合。

### 參考答案（q4.c）
```c
// Q4：用 open() 指定權限 0640 建立 score.txt，把 1~5 的平方寫進檔案（一行一個）
// 只能用 <unistd.h> 與 <fcntl.h>
#include <unistd.h>
#include <fcntl.h>

void my_fprint_int(int fd, int a) {      // 印到「任意 fd」：1=螢幕，或 open 回傳的檔案
    char buf[12];
    int i = 11;
    unsigned int u = (a < 0) ? -(unsigned int)a : (unsigned int)a;
    do { buf[i--] = '0' + u % 10; u /= 10; } while (u > 0);
    if (a < 0) buf[i--] = '-';
    write(fd, buf + i + 1, 11 - i);
}

int main() {
    int fd = open("score.txt", O_CREAT | O_WRONLY | O_TRUNC, 0640);
    if (fd == -1) {
        write(2, "open failed\n", 12);
        return 1;
    }
    for (int k = 1; k <= 5; k++) {
        my_fprint_int(fd, k * k);
        write(fd, "\n", 1);
    }
    close(fd);
    write(1, "done\n", 5);
    return 0;
}
```

### ✅ 已驗證
```
$ gcc -Wall -Wextra -Werror=implicit-function-declaration q4.c -o q4
$ ./q4
done
$ ls -l score.txt
-rw-r----- 1 root root 12 Oct 27 12:01 score.txt
$ cat score.txt
1
4
9
16
25
```

### 容易踩的坑
1. **umask**：open 實際權限 = `mode & ~umask`。預設 umask 是 `022`，所以 `0666` 會變 `0644`、`0777` 會變 `0755`。題目要 `0640` 沒受影響；但若題目要 `0666`，你會看到 `rw-r--r--`——口頭說明「被 umask 022 遮掉」即可。
2. **檔案已存在時 mode 不會改**：open 的 mode 只在「新建」時生效。重測前先 `rm score.txt`，否則看到的是舊權限。
3. `0640` 不能寫成 `640`（十進位 640 = 八進位 1200，完全不同）。
4. 數字寫進檔案要轉成字元，`write(fd, &x, sizeof(int))` 會寫進 4 個二進位 bytes，`cat` 出來是亂碼。

---

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

---

## 第 6 題：用 C library（fopen/fread/fwrite/fclose）改寫檔案複製

### 題目
把第 3 題的 mycp 改用 `fopen/fread/fwrite/fclose` 實作，開檔失敗用 `perror` 印錯誤並 `exit(1)`。完成後回答：兩個版本用 `strace -c` 看，誰的 read/write 次數多？

**輸入／輸出範例**
```
$ ./q6 big.bin d6.bin
copied 10000 bytes
```

### 變形自哪裡／考什麼
- **example3（p.26~28）**：fwrite_file.c（`"w+"`、`perror`、`exit(1)`）＋ fread_file.c（`"rb"`、`fread(buffer, 1, sizeof(buffer), fp)`）。
- **p.22~25** 四個函式的參數與回傳值（fopen 失敗回 `NULL`；fread 回傳「元素個數」）。

### 可能性：低
沒有「只准用 unistd.h」那種限制的挑戰性，太像照抄 example3；比較可能當成第 3 題的追加問題。

### 參考答案（q6.c）
```c
// Q6：用 C library (fopen/fread/fwrite/fclose) 改寫檔案複製
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s <src> <dst>\n", argv[0]);
        return 1;
    }
    FILE *in = fopen(argv[1], "rb");
    if (in == NULL) { perror("fopen src"); exit(1); }
    FILE *out = fopen(argv[2], "wb");
    if (out == NULL) { perror("fopen dst"); fclose(in); exit(1); }

    char buffer[100];
    size_t n;
    long total = 0;
    while ((n = fread(buffer, 1, sizeof(buffer), in)) > 0) {  // 回傳「元素個數」
        fwrite(buffer, 1, n, out);
        total += n;
    }
    fclose(in);
    fclose(out);
    printf("copied %ld bytes\n", total);
    return 0;
}
```

### ✅ 已驗證
```
$ gcc -Wall -Wextra q6.c -o q6
$ ./q6 big.bin d6.bin && cmp big.bin d6.bin && echo SAME
copied 10000 bytes
SAME
```

### 容易踩的坑
1. `fread(buffer, sizeof(buffer), 1, in)`（size 和 count 對調）：回傳值變成 0 或 1，最後不滿一塊時會回 0 → 尾巴被吃掉。要用 `fread(buffer, 1, sizeof(buffer), in)`。
2. fopen 的 mode：`"w"` 會清空、`"r"` 不存在會失敗；二進位檔加 `b`（Linux 上沒差，但寫了比較標準）。
3. `fopen` 失敗回傳 **`NULL`**（不是 -1）；`open` 失敗才是 -1。
4. 本題 buffer 只開 100，但 strace 還是只看到少量 read，因為 stdio 自己的 buffer 是 4096。

---

## 考前 5 分鐘速查

**只能用 `<unistd.h>` 時**
- 能用：`read write close`（`open` 的 flag 要 `<fcntl.h>`）。不能用：`printf scanf strlen atoi`。
- 印整數模板：`unsigned u = a<0 ? -(unsigned)a : a;` → `do { buf[i--]='0'+u%10; u/=10; } while(u);` → 負號 → `write(1, buf+i+1, 長度)`。
- 讀整數模板：`read(0,&c,1)` 一次一字元；跳空白 → 判 `'-'` → `val = val*10 + (c-'0')`；**回傳 0 = EOF**。
- 測資必備：`0`、`-2147483648`、`2147483647`。

**System call I/O（p.13~16）**
```c
int fd = open("f.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);  // 失敗 -1
ssize_t n = read(fd, buf, sizeof(buf));   // >0 讀到 n 個、0 = EOF、-1 錯誤；不補 '\0'
write(fd, buf, n);                        // 寫 n 個，不是 sizeof(buf)
close(fd);                                // 成功 0
```
- 讀：`O_RDONLY`；寫：`O_WRONLY`；讀寫：`O_RDWR`；`O_CREAT` 不存在就建；`O_TRUNC` 清空；`O_APPEND` 附加。
- fd：**0 stdin、1 stdout、2 stderr**（程式啟動時 OS 自動打開）。

**C library I/O（p.22~25）**
```c
FILE *fp = fopen("f.txt", "w");           // 失敗 NULL；r / w / a / r+ / w+ / a+
size_t k = fread(buf, 1, sizeof(buf), fp);  // 回傳「元素個數」
fwrite(buf, 1, k, fp);
fclose(fp);                               // 會 flush；失敗回 EOF
```

**權限碼（p.10~12）**
- r=4、w=2、x=1；owner / group / other 各一位。`rwxr-xr-x`=755、`rw-r--r--`=644、`rw-r-----`=640、`-w-r--r--`=244。
- 程式裡一定寫 **`0644`**（開頭 0 = 八進位）。實際權限 = mode & ~umask（預設 022）。
- `ls -l` 第一個字元：`-` 一般檔、`d` 目錄。

**strace / example4（p.9、p.29~31）**
- `strace -c ./prog`：統計 syscall 次數（輸出在 stderr）。
- read() 一次 1 byte 讀 10MB → **10,485,762 次 read**；fread() → **2,562 次**（≈10MB/4096）。
- 原因：fread/fwrite 在 user space 有 buffer，一次跟 kernel 要 4096 bytes；每次 syscall 都要切換 user mode ↔ kernel mode，次數少就快 → **這就是偏好使用包裝後函式的理由**（Exercise 學習目標）。

**分層（p.5）**：User application → Library → System call interface → OS kernel（上兩層 user space，下面 kernel space）。POSIX 由 IEEE 制定，規範介面與行為，不規定 kernel 實作。
