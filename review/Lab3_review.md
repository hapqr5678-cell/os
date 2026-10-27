# Lab 3 複習：System Call vs APIs

> 來源：Lab3.pptx（33 張投影片，含講者備註）＋ practice/lab3/ex.c

---

## 1. 這個 LAB 在學什麼

認識 Unix-like 系統與 **POSIX** 標準、user space／kernel space 的分層，學會用 **system call I/O**（`open/read/write/close`）和 **C library I/O**（`fopen/fread/fwrite/fclose`）讀寫檔案，並用 `strace` 實驗證明：**`fread()` 因為有 user-space buffer，system call 次數遠少於 `read()`，所以快很多**。另外介紹檔案權限碼（`ls -l`、`chmod`）。Exercise：只用 `<unistd.h>` 印出整數。

---

## 2. 核心觀念

### 2.1 Unix-like 系統
- 與 Unix 設計理念相似的 OS：**Linux**（嚴格說 Linux 只是 kernel；Ubuntu、Debian 等是發行版）、**macOS**、**BSD 系列**（FreeBSD、OpenBSD、NetBSD）。
- 特色：**一切都是檔案**、命令列操作（CLI）、多使用者多工、process 管理、Shell 介面、開放並遵守 **POSIX**。

### 2.2 POSIX
- **Portable Operating System Interface**，由 **IEEE** 制定。
- 規範 Unix／Unix-like 系統**對外提供的程式介面與行為**，讓程式容易在不同系統間移植。
- 重要觀念（備註）：**POSIX 規定 API「該有什麼功能和行為」，不規定 kernel 內部怎麼實作。**
- 規範內容：① System interfaces（`read()`、`write()`、`fork()`、`wait()`…）② Library（含大部分 C 標準函式庫 `stdio.h`、`stdlib.h`）③ Shell 與工具 ④ 其他。
- 投影片表格列出整學期會教的 POSIX 函式，括號數字是 x86-64 syscall 編號，例如 `read(0) write(1) open(2) close(3) fork(57) getpid(39) exec(59) wait(61) kill(62) mmap(9)`。有編號的是 system call；沒有編號的（`fopen`、`mq_open`、`pthread_create`、`sem_init`…）是 library 函式。

### 2.3 分層架構（投影片第 5 頁）
```
 User application
        │
     Library（例如 C library）           ← user space
 ───────┼──── System call interface ────
     OS kernel                           ← kernel space
```
- User application 不能直接操作 kernel 資源，需要 OS 服務就要經過 **system call interface**。
- 一般透過 library 函式呼叫；library 幫忙處理一些事，必要時才發 system call 進 kernel。

### 2.4 System call I/O vs C library I/O（重點比較）
| | System call I/O | C Library I/O |
|---|---|---|
| 函式 | `open read write close` | `fopen fread fwrite fclose` |
| 標頭 | `<unistd.h>`、`<fcntl.h>` | `<stdio.h>` |
| 代表檔案的東西 | **file descriptor（int）** | **`FILE *`**（stream） |
| 緩衝 | 無 user-space buffer，每次呼叫都進 kernel | 有 user-space buffer，攢滿一批才呼叫一次 `read()`/`write()` |
| 錯誤回傳 | -1 並設定 `errno` | `fopen` 回 `NULL`；`fclose` 回 `EOF` |
| 可移植性 | POSIX 系統 | 任何有 C 標準函式庫的平台 |

**為什麼 fread 比 read 快？**（example4，必考）
- 讀 10MB 檔，一次讀 1 byte、讀 10M 次：
  - `read()` 版：`strace -c` 顯示 **10,485,762 次 read syscall**，花了 663 秒（在 strace 下）。
  - `fread()` 版：只有 **2,562 次 read syscall**，約 1.5 秒。
- 原因：`fread()` 第一次發現 buffer 空了，就呼叫一次 `read(fd, buf, 4096)` 從 kernel 拿 4096 bytes 放到 user-space buffer，之後 4096 次 `fread()` 都直接從 buffer 拿，不必進 kernel。**每次 system call 都要切換 user mode ↔ kernel mode，成本很高**，所以減少 syscall 次數就變快。
- 這就是 Exercise 的學習目標：**了解為何偏好使用把 system call 包裝過的函式。**

### 2.5 File Descriptor
- `open()` 回傳的小整數，代表一個開啟的檔案。
- 每個程式啟動時 OS 自動開好三個：**0 = stdin、1 = stdout、2 = stderr**。stdout 預設連到 terminal，所以 `write(1, ...)` 就能印到螢幕。

### 2.6 權限碼（Permission Code）
`ls -l` 輸出：
```
-rw-r--r-- 1 owner group 13 Sep 12 04:52 hello.txt
│└┬┘└┬┘└┬┘
│ │  │  └ Other
│ │  └ Group
│ └ Owner
└ 類型：- 一般檔案、d 目錄
```
- `r`=4（讀）、`w`=2（寫）、`x`=1（執行），每組加總成一個八進位數字。
- `rwxr-xr-x` = 7 5 5 = `755`；`rw-r--r--` = `644`；`-w-r--r--` = `244`。
- 投影片示範：`chmod 0244 hello.txt` 後 owner 只剩寫入權，`cat hello.txt` → `Permission denied`。
- `open()` 的 mode 參數 `0644` 前面的 **0 表示八進位**。

### 2.7 strace
攔截並顯示程式呼叫的 system calls 與 signals。`strace -c ./prog` 會統計每種 syscall 的次數（calls）、平均耗時（usecs/call）等。
連最簡單的 `printf("Hello world\n")` 程式，`strace -c` 也看到 `execve, mmap, openat, write...` 共 34 次 syscall，其中真正輸出的只有 1 次 `write`。

---

## 3. 指令／API 速查表

| 名稱 | 用途 | 重要參數／回傳值 | 範例 |
|---|---|---|---|
| `int open(const char *filename, int flags[, mode_t mode])` | 開檔，取得 fd | flags：`O_RDONLY` `O_WRONLY` `O_RDWR` `O_CREAT` `O_TRUNC` `O_APPEND`（可用 `\|` 組合）；有 `O_CREAT` 時要給 mode。回傳 fd，失敗 -1 + errno | `open("hello.txt", O_CREAT\|O_WRONLY\|O_TRUNC, 0644)` |
| `ssize_t read(int fd, void *buf, size_t count)` | 讀最多 count bytes | 回傳實際讀到的 bytes（≥0），**0 = EOF**，-1 = 錯誤 | `read(f1, buffer, 4)` |
| `ssize_t write(int fd, const void *buf, size_t count)` | 寫 count bytes | 回傳實際寫入數（可能少於 count），-1 錯誤 | `write(1, "hi\n", 3)` |
| `int close(int fd)` | 關閉 fd | 0 成功，-1 錯誤 | `close(f1)` |
| `FILE *fopen(const char *filename, const char *mode)` | 開檔 | mode：`r` `w` `a` `r+` `w+` `a+`；失敗回 `NULL` | `fopen("XXX.txt","w")` |
| `size_t fread(void *ptr, size_t size, size_t count, FILE *stream)` | 讀 count 個 size 大小的元素 | 回傳**成功讀到的元素個數** | `fread(buffer, 1, sizeof(buffer), f1)` |
| `size_t fwrite(const void *ptr, size_t size, size_t count, FILE *stream)` | 寫 | 回傳成功寫入的元素個數 | `fwrite(buffer, sizeof(char), 4, f1)` |
| `int fclose(FILE *stream)` | 關閉並 flush buffer | 0 成功，失敗回 `EOF` | `fclose(f1)` |
| `ls -l` | 長格式列檔 | 權限、擁有者、大小、時間 | `ls -l hello.txt` |
| `chmod` | 改權限 | 八進位或 `u+x` | `chmod 0244 hello.txt` |
| `strace -c ./prog` | 統計 syscall | calls、usecs/call | `strace -c ./helloworld` |
| `sudo sh -c "echo 3 > /proc/sys/vm/drop_caches"` | 清掉 page cache，讓計時公平 | — | example4 |

fopen mode 細節：
| mode | 讀/寫 | 檔案不存在 | 檔案存在 |
|---|---|---|---|
| `"r"` | 讀 | 失敗 | 從頭讀 |
| `"w"` | 寫 | 建立 | **截成 0（清空）** |
| `"a"` | 附加寫 | 建立 | 寫在尾端 |
| `"r+"` | 讀寫 | 失敗 | 不截斷 |
| `"w+"` | 讀寫 | 建立 | 截斷 |
| `"a+"` | 讀寫 | 建立 | 寫入一律在尾端 |

---

## 4. 程式範例逐行解說

### 4.1 example1 – write_file.c
```c
#include <unistd.h>     // open(), read(), write(), close()
#include <fcntl.h>      // O_CREAT, O_WRONLY ...
int main() {
    int fd;
    char buffer[] = "Hello World\n";
    fd = open("hello.txt", O_CREAT | O_WRONLY, 0644);
    write(fd, buffer, sizeof(buffer));   // 13 bytes（含結尾 '\0'）
    close(fd);
    return 0;
}
```
- `sizeof(buffer)` = 12 個字元 + `'\0'` = **13**，所以檔案大小 13 bytes（`ls -l` 顯示 13），**檔案裡其實多寫了一個 `'\0'`**。
- 投影片註解寫 `O_RDWR -> open for write only`，但程式碼用的是 `O_WRONLY`——註解有誤，以程式碼為準。
- 沒加 `O_TRUNC`：若 hello.txt 原本比較長，舊內容後半會殘留。

### 4.2 example1 – read_file.c
```c
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
int main() {
    int fd;
    char buffer[100];
    fd = open("hello.txt", O_RDONLY);
    read(fd, buffer, sizeof(buffer));   // 最多讀 100 bytes，實際讀到 13
    printf("%s", buffer);
    close(fd);
    return 0;
}
```
輸出 `Hello World`。能正常印是因為 write_file 剛好把 `'\0'` 也寫進檔案；**`read()` 不會自動補 `'\0'`**，一般檔案要自己用回傳值 `n` 補 `buffer[n] = '\0'`。

### 4.3 example2 – write_to_terminator.c
```c
#include <unistd.h>
int main(){
    char buffer[] = "Hello World\n";
    write(1, buffer, sizeof(buffer));   // fd 1 = stdout
    return 0;
}
```
不用 `<stdio.h>` 也能印到螢幕：直接寫到 fd 1。

### 4.4 example3 – fwrite_file.c / fread_file.c
```c
FILE *fp = fopen("hello.txt", "w+");          // 讀寫，不存在就建，存在就清空
if (fp == NULL) { perror("fopen"); exit(1); }
fwrite(buffer, sizeof(buffer), 1, fp);        // 1 個大小為 13 的元素
fclose(fp);                                   // flush buffer 並關閉
```
```c
FILE *fp = fopen("hello.txt", "rb");          // 唯讀（binary）
size_t n = fread(buffer, 1, sizeof(buffer), fp);   // n = 13
printf("%s", buffer);
fclose(fp);
```
- `fwrite(buffer, sizeof(buffer), 1, fp)` 回傳 **1**（元素個數），不是 13。
- `fread(buffer, 1, 100, fp)` 回傳 **13**（每個元素 1 byte）。
- 這就是 size／count 順序的差別：回傳值單位是「元素」。

### 4.5 example4 – read() vs fread()
1. `create_file` 建一個 10MB（10485760 bytes）的 my_file.txt。
2. `sequential_read`（用 read，每次 1 byte）vs `sequential_fread`（用 fread，每次 1 byte），每次執行前清 cache。
3. 結果：read 版 2.94 秒、fread 版 0.14 秒；`strace -c` 顯示 read 版 10,485,762 次 `read`，fread 版只有 2,562 次。

### 4.6 學生作業 practice/lab3/ex.c（只用 unistd.h 印整數）
```c
#include <unistd.h>
void myp(long long a) {
  char buffer[18];
  buffer[18] = '\n';          // ← BUG：索引 18 超出陣列（合法 0~17）
  int neg = 0;
  int i = 17;
  if(a < 0) { a = a * (-1); neg = 1; }
  else if(a == 0) { buffer[i--] = '0'; }
  while(a > 0) { buffer[i--] = '0' + a%10; a = a/10; }   // 從尾巴往前填數字
  if(neg == 1) { buffer[i--] = '-'; }
  write(1, buffer + i + 1, 18 - i);   // 從第一個字元寫到 buffer[18]（含換行）
}
int main() {
  myp(0); myp(100); myp(2147483647); myp(-2147483648); myp(-1644604641554687);
}
```
**思路**：用 `a%10` 取個位數、`a/10` 去掉個位，從陣列尾端往前填，最後一次 `write` 輸出。負數先轉正、最後補 `-`；0 要特判（否則 while 一次都不跑）。

預期輸出：
```
0
100
2147483647
-2147483648
-1644604641554687
```

**問題與 bug：**
1. **陣列越界（未定義行為）**：`char buffer[18]` 合法索引是 0~17，但程式寫入 `buffer[18] = '\n'`，而且 `write(..., 18 - i)` 也會讀到 `buffer[18]`。能跑出正確結果純屬運氣（剛好沒蓋到重要資料）；換編譯器或開 `-O2`／`-fsanitize=address` 就可能壞掉。
2. **容量不足的風險**：`long long` 最小值 `-9223372036854775808` 有 19 位數 + 負號 + 換行 = 21 字元，18 格放不下。
3. **`LLONG_MIN` 取負號會溢位**：`a * (-1)` 對 `-9223372036854775808` 是 signed overflow（UB）。題目測資沒測到，但值得知道。
4. `main` 沒寫 `return 0;`（C99 起 main 會自動回 0，不算錯）。

**修正版（參考解法）：**
```c
#include <unistd.h>

void my_print(long long a) {
    char buf[21];                       /* 19 位數 + '-' + '\n' */
    int i = 20;
    unsigned long long u;
    buf[i--] = '\n';
    if (a < 0) u = (unsigned long long)(-(a + 1)) + 1;   /* 避免 LLONG_MIN 溢位 */
    else       u = (unsigned long long)a;
    if (u == 0) buf[i--] = '0';
    while (u > 0) {
        buf[i--] = '0' + (u % 10);
        u /= 10;
    }
    if (a < 0) buf[i--] = '-';
    write(1, buf + i + 1, 20 - i);      /* 從 buf[i+1] 寫到 buf[20] */
}

int main(void) {
    my_print(0);
    my_print(100);
    my_print(2147483647);
    my_print(-2147483648LL);
    return 0;
}
```
助教會檢查有沒有偷 `#include <stdio.h>`，所以**整個檔案只能有 `<unistd.h>`**（不能用 `printf`、`sprintf`、`itoa`）。

---

## 5. 上機考實作步驟

```bash
# 寫檔 / 讀檔範例
vim write_file.c
vim read_file.c
gcc write_file.c -o write_file
gcc read_file.c -o read_file
./write_file
./read_file                     # Hello World
ls -l hello.txt                 # -rw-r--r-- ... 13 ... hello.txt

# 權限實驗
chmod 0244 hello.txt
ls -l hello.txt                 # --w-r--r--
cat hello.txt                   # Permission denied
chmod 644 hello.txt             # 改回來

# 比較 read vs fread
sudo sh -c "echo 3 > /proc/sys/vm/drop_caches"
./sequential_read 1
sudo sh -c "echo 3 > /proc/sys/vm/drop_caches"
./sequential_fread 1
strace -c ./sequential_read 1
strace -c ./sequential_fread 1

# Exercise
vim lab3.c                      # 只 include <unistd.h>
gcc lab3.c -o lab3
./lab3
grep include lab3.c             # 自我檢查：只能看到 unistd.h
```

---

## 6. 常見錯誤與陷阱

1. `open()` 有 `O_CREAT` 卻沒給第三個參數 mode → 新檔權限是亂的。
2. mode 寫成 `644`（十進位）而不是 `0644`（八進位）→ 權限完全不對。
3. 以為 `read()` 會自動補 `'\0'` → 印字串時印出垃圾。
4. 忘了 `read()` 回傳 **0 代表 EOF**，-1 才是錯誤。
5. `write()` 回傳值可能小於 count（部分寫入）。
6. `fread/fwrite` 的回傳值是「**元素個數**」，不是 byte 數。
7. 用 `fwrite` 後沒 `fclose`/`fflush` 就結束（例如程式 crash）→ buffer 內資料沒真的寫入檔案。
8. `"w"` 模式會**清空**既有檔案。
9. 混用 `printf` 與 `write(1, ...)`：printf 有 buffer、write 沒有，輸出順序可能跟程式碼順序不同（補充）。
10. Exercise 的陣列大小與索引邊界（見 4.6）。

---

## 7. 筆試模擬題

**Q1（選擇）** POSIX 是由哪個組織制定？主要目的是？
(A) ISO／定義 C 語法　(B) IEEE／讓程式在不同 Unix-like 系統間可移植　(C) Linux Foundation／規定 kernel 實作　(D) GNU／提供 shell
> **答：B**。POSIX 規範介面與行為，不規定 kernel 內部實作。

**Q2（是非）** POSIX 規定了 `read()` 在 kernel 內部必須如何實作。
> **答：錯**。只規定功能與行為。

**Q3（選擇）** 程式啟動時 fd 2 預設是？
(A) stdin　(B) stdout　(C) stderr　(D) 第一個 open 的檔案
> **答：C**。0=stdin、1=stdout、2=stderr。

**Q4（簡答）** 為何用 `fread()` 一次讀 1 byte 讀 10MB，比 `read()` 一次讀 1 byte 快很多？
> **答：** `fread()` 有 user-space buffer，一次 `read()` syscall 就從 kernel 拿約 4096 bytes，之後的 fread 直接從 buffer 取，syscall 次數從約一千萬次降到幾千次；每次 syscall 都需 user/kernel mode 切換，成本高。

**Q5（輸出預測）**
```c
char buffer[] = "Hello World\n";
int fd = open("a.txt", O_CREAT|O_WRONLY|O_TRUNC, 0644);
ssize_t n = write(fd, buffer, sizeof(buffer));
printf("%zd\n", n);
```
> **答：** `13`（12 個字元 + `'\0'`）。

**Q6（權限）** `chmod 754 file` 之後，`ls -l` 權限欄位顯示？
> **答：** `-rwxr-xr--`（7=rwx、5=r-x、4=r--）。

**Q7（選擇）** `read()` 回傳 0 表示？
(A) 錯誤　(B) 讀到 EOF　(C) 讀到 1 byte　(D) 檔案不存在
> **答：B**。錯誤是 -1。

**Q8（輸出預測）** 檔案 data.txt 內容為 `ABCDE`（5 bytes，無換行）：
```c
FILE *fp = fopen("data.txt", "r");
char buf[10];
size_t n = fread(buf, 2, 4, fp);
printf("%zu\n", n);
```
> **答：** `2`。要讀 4 個 2-byte 元素，但只有 5 bytes，只湊得出 2 個完整元素（第 5 個 byte 不足一個元素）。

**Q9（選擇）** 想「開檔寫入，不存在就建立，存在就清空」，用 `open` 應該怎麼寫 flags？
(A) `O_WRONLY`　(B) `O_WRONLY|O_APPEND`　(C) `O_CREAT|O_WRONLY|O_TRUNC`　(D) `O_RDONLY|O_CREAT`
> **答：C**。對應 `fopen` 的 `"w"`。

**Q10（簡答）** `fopen` 的 `"a+"` 和 `"w+"` 差在哪？
> **答：** 兩者都可讀寫、檔案不存在會建立。`"w+"` 會把既有檔案清空；`"a+"` 不清空，而且寫入一律附加在檔尾。

**Q11（選擇）** 下列哪一個「不是」system call？
(A) `write`　(B) `fork`　(C) `fopen`　(D) `close`
> **答：C**。`fopen` 是 C library 函式，內部呼叫 `open`。

**Q12（程式）** 只用 `<unistd.h>`，寫出把字元 `'A'` 和換行印到螢幕的一行程式。
> **答：** `write(1, "A\n", 2);`

**Q13（簡答）** `strace` 是做什麼的？`-c` 的作用？
> **答：** 追蹤程式執行時呼叫的 system call 和收到的 signal；`-c` 會統計每種 syscall 的次數、耗時並在結束時列表。
