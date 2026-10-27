# Lab4_g2

> 來源:review/predict/Lab4_predict.md

## 題 2：統計函式庫 max / min / average（多個 .o 包成一個 library）

### 題目
1. 撰寫 `max.c`、`min.c`、`average.c` 三個檔案，各自實作一個函式，參數為陣列位址與長度；共用標頭 `mylib.h`。
2. `main.c` 產生 5 個不重複隨機整數（1~100），印出陣列、最大值、最小值、平均（小數點後 2 位）。
3. 把三個 `.o` 包成 `mylib.a`，編成 `stat_static`；再包成 `mylib.so`，編成 `stat_dynamic`。
4. `ls -l` 比較兩個執行檔大小，並用 `ldd` 確認 `stat_dynamic` 用到 `mylib.so`。

輸出範例：
```
Array: 94 75 82 29 9
Max = 94
Min = 9
Average = 57.80
```

### 變形自哪裡／考點
- 結構完全仿照 **p.5 範例**（`add.c` + `sub.c` + `lib.h` → 一個 library），內容換成 Exercise 風格的陣列操作。
- 考 `gcc -c` 一次編多檔、`ar rcs` 包多個 .o（p.11）、`-shared` 合併多個 .o（p.21）、`ldd`（p.24）。

### 可能性：高
Exercise 只有一個 .o，太單薄；把範例的「多個 .c」和 Exercise 的「陣列」合體是最自然的升級（出題邏輯 3：串起所有小節）。

### 參考答案
**mylib.h**
```c
int max(int *arr, int n);
int min(int *arr, int n);
double average(int *arr, int n);
```
**max.c**
```c
#include "mylib.h"

int max(int *arr, int n) {
    int m = arr[0];
    for (int i = 1; i < n; i++)
        if (arr[i] > m) m = arr[i];
    return m;
}
```
**min.c**
```c
#include "mylib.h"

int min(int *arr, int n) {
    int m = arr[0];
    for (int i = 1; i < n; i++)
        if (arr[i] < m) m = arr[i];
    return m;
}
```
**average.c**
```c
#include "mylib.h"

double average(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++)
        sum += arr[i];
    return (double)sum / n;          /* 先轉 double 再除，否則整數除法會截掉小數 */
}
```
**main.c**
```c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mylib.h"

int main(void) {
    int arr[5], cnt = 0;
    srand(time(NULL));
    while (cnt < 5) {
        int x = rand() % 100 + 1, dup = 0;
        for (int k = 0; k < cnt; k++)
            if (arr[k] == x) { dup = 1; break; }
        if (!dup) arr[cnt++] = x;
    }
    printf("Array: ");
    for (int i = 0; i < 5; i++) printf("%d ", arr[i]);
    printf("\nMax = %d\n", max(arr, 5));
    printf("Min = %d\n", min(arr, 5));
    printf("Average = %.2f\n", average(arr, 5));
    return 0;
}
```
**指令序列**
```bash
# Static
gcc -c max.c min.c average.c
ar rcs mylib.a max.o min.o average.o
ar t mylib.a                                   # （可選）列出 archive 內容，p.10 usage 有 t
gcc main.c -L. mylib.a -o stat_static
./stat_static

# Dynamic
gcc -c -fPIC max.c min.c average.c
gcc max.o min.o average.o -shared -o mylib.so
gcc main.c -L. mylib.so -o stat_dynamic
sudo cp mylib.so /lib/mylib.so
./stat_dynamic
ldd stat_dynamic
ls -l
```

### ✅ 已驗證
```
$ ar t mylib.a
max.o
min.o
average.o
$ ./stat_static
Array: 94 75 82 29 9
Max = 94
Min = 9
Average = 57.80
$ LD_LIBRARY_PATH=. ./stat_dynamic
Array: 94 75 82 29 9
Max = 94
Min = 9
Average = 57.80
$ ls -l stat_static stat_dynamic mylib.a mylib.so
-rw-r--r-- 1 root root  4216 mylib.a
-rwxr-xr-x 1 root root 15232 mylib.so
-rwxr-xr-x 1 root root 16208 stat_dynamic
-rwxr-xr-x 1 root root 16304 stat_static        ← 比 dynamic 大 96 bytes
$ ldd stat_dynamic
	linux-vdso.so.1 (...)
	mylib.so => ./mylib.so (...)               ← 考場 cp 到 /lib 後會顯示 /lib/mylib.so
	libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (...)
	/lib64/ld-linux-x86-64.so.2 (...)
$ ldd stat_static                              ← static 版看不到 mylib
	linux-vdso.so.1 (...)
	libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (...)
	/lib64/ld-linux-x86-64.so.2 (...)
```

### 容易踩的坑
- **`ar rcs` 第一個參數是 archive 名稱**：寫成 `ar rcs max.o mylib.a` 會出錯（實測：`ar: max.o: file format not recognized`），更糟的情況是把 .o 檔弄壞。
- `-shared` 要**把所有 .o 都列上**，漏一個就會在連結 main 時 `undefined reference`。
- `average` 用 `sum / n` 會是整數除法（57 而不是 57.80），要 `(double)sum / n`。
- 標頭檔的宣告型別要和實作一致（`double average`），否則 `conflicting types`。
- 每個 .c 都 `#include "mylib.h"`，main.c 用雙引號 `"mylib.h"` 不是 `<mylib.h>`。
