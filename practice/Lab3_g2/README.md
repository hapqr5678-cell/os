# Lab3_g2

> 來源:review/predict/Lab3_predict.md

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
