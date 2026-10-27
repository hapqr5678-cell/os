# Lab3_g1

> 來源:review/predict/Lab3_predict.md

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
