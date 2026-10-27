# Lab4_g5

> 來源:review/predict/Lab4_predict.md

## 題 5：擴充投影片範例 — 四則運算 library

### 題目
把 p.5 範例改成兩個參數的四則運算：`add.c`、`sub.c`、`mul.c`、`div.c`（函式 `add / sub / mul / divide`），共用 `lib.h`。`main.c` 讀入兩個整數並印出四則運算結果（除法到小數點後 2 位，除數為 0 要印提示）。分別以 static（`calc_static`）、dynamic（`calc_dynamic`）編譯並 `ls -l`。

輸出範例：
```
Input two integers: 7 2
7 + 2 = 9
7 - 2 = 5
7 * 2 = 14
7 / 2 = 3.50
```

### 變形自哪裡／考點
- 直接改 **p.5 範例**（add/sub）＋ p.11 / p.21 的指令（檔名 `lib.a`、`lib.so` 都和簡報一樣）。

### 可能性：中
範例程式和指令都完全照簡報，助教要出「不像 Exercise 原題」的題目時很方便；但沒有陣列/指標，難度偏低，較可能是題組中的小題。

### 參考答案
**lib.h**
```c
int add(int a, int b);
int sub(int a, int b);
int mul(int a, int b);
double divide(int a, int b);
```
**add.c / sub.c / mul.c / div.c**
```c
// add.c
#include "lib.h"
int add(int a, int b) { return a + b; }

// sub.c
#include "lib.h"
int sub(int a, int b) { return a - b; }

// mul.c
#include "lib.h"
int mul(int a, int b) { return a * b; }

// div.c
#include "lib.h"
double divide(int a, int b) { return (double)a / b; }
```
**main.c**
```c
#include <stdio.h>
#include "lib.h"

int main(void) {
    int a, b;
    printf("Input two integers: ");
    scanf("%d %d", &a, &b);
    printf("%d + %d = %d\n", a, b, add(a, b));
    printf("%d - %d = %d\n", a, b, sub(a, b));
    printf("%d * %d = %d\n", a, b, mul(a, b));
    if (b != 0)
        printf("%d / %d = %.2f\n", a, b, divide(a, b));
    else
        printf("%d / %d = undefined (divide by zero)\n", a, b);
    return 0;
}
```
**指令序列**
```bash
gcc -c add.c sub.c mul.c div.c
ar rcs lib.a add.o sub.o mul.o div.o
gcc main.c -L. lib.a -o calc_static
./calc_static

gcc -c -fPIC add.c sub.c mul.c div.c
gcc add.o sub.o mul.o div.o -shared -o lib.so
gcc main.c -L. lib.so -o calc_dynamic
sudo cp lib.so /lib/lib.so
./calc_dynamic
ldd calc_dynamic
ls -l
```

### ✅ 已驗證
```
$ ar t lib.a
add.o
sub.o
mul.o
div.o
$ echo "7 2" | ./calc_static
Input two integers: 7 + 2 = 9
7 - 2 = 5
7 * 2 = 14
7 / 2 = 3.50
$ echo "7 0" | LD_LIBRARY_PATH=. ./calc_dynamic
Input two integers: 7 + 0 = 7
7 - 0 = 7
7 * 0 = 0
7 / 0 = undefined (divide by zero)
$ ls -l calc_static calc_dynamic lib.a lib.so
-rwxr-xr-x 1 root root 16176 calc_dynamic
-rwxr-xr-x 1 root root 16296 calc_static      ← 比 dynamic 大 120 bytes（4 個函式都被複製進來）
-rw-r--r-- 1 root root  5292 lib.a
-rwxr-xr-x 1 root root 15280 lib.so
$ ldd calc_dynamic | grep lib.so
	lib.so => ./lib.so (...)
```

### 容易踩的坑
- **函式不要取名 `div`**：`<stdlib.h>` 已有 `div()`，實測 `error: conflicting types for 'div'`。用 `divide` 或 `my_div`。
- 若投影片範例的 `lib.so` 之前已 cp 到 `/lib/lib.so`（舊版只有 add/sub），**新版一定要再 cp 覆蓋**，否則執行時會出錯（實測：`symbol lookup error: ./calc_dynamic: undefined symbol: divide`）。
- 除法 `(double)a / b`，`a / b` 是整數除法。
- 除以 0 要先判斷，整數除 0 會 `Floating point exception`。
