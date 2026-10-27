# Lab3_g3

> 來源:review/predict/Lab3_predict.md

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
