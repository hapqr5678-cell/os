# Lab4_g1

> 來源:review/predict/Lab4_predict.md

## 題 1：降冪排序 library（10 個不重複亂數）

### 題目
1. `sort.c`：建立 `sort_desc` 函式，參數接受**陣列位址與長度**，將陣列由**大到小**排序。
2. `exercise.c`：產生 **10 個不重複**、範圍 1~100 的隨機整數，印出排序前／後。
3. 將 `sort.o` 加入 `exlib.a`，編成 `exercise1`（static）。
4. 用 shared library 方式編成 `exercise2`，執行成功後用 `ls -l` 比較檔案大小。

輸出範例：
```
$ ./exercise1
Before sort: 61 67 88 54 4 30 36 8 84 71
After sort: 88 84 71 67 61 54 36 30 8 4
```

### 變形自哪裡／考點
- Exercise 1（p.14~15）＋ Exercise 2（p.26~27），只把「由小到大、5 個」改成「由大到小、N 個、多傳長度」。
- 考 static 流程（p.11）、dynamic 流程（p.21~23）、`ls -l` 比大小（p.26）。

### 可能性：高
出題邏輯 1（同機制換小題目）最直接的做法；助教只需改兩三個字就能出一題新題目，改分也最好比對。

### 參考答案
**exlib.h**
```c
void sort_desc(int *arr, int n);
```
**sort.c**
```c
#include "exlib.h"

void sort_desc(int *arr, int n) {
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - 1 - i; j++)
            if (arr[j] < arr[j + 1]) {      /* 前面比後面小就交換 → 由大到小 */
                int t = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = t;
            }
}
```
**exercise.c**
```c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "exlib.h"

#define N 10

int main(void) {
    int arr[N], cnt = 0;
    srand(time(NULL));
    while (cnt < N) {                      /* 產生 N 個不重複 1~100 */
        int x = rand() % 100 + 1, dup = 0;
        for (int k = 0; k < cnt; k++)
            if (arr[k] == x) { dup = 1; break; }
        if (!dup) arr[cnt++] = x;
    }
    printf("Before sort: ");
    for (int i = 0; i < N; i++) printf("%d ", arr[i]);
    sort_desc(arr, N);                     /* 傳陣列位址 + 長度 */
    printf("\nAfter sort: ");
    for (int i = 0; i < N; i++) printf("%d ", arr[i]);
    printf("\n");
    return 0;
}
```
**指令序列**
```bash
# ---- Static（Exercise 1 流程，p.11 / p.15）----
gcc -c sort.c                              # → sort.o
ar rcs exlib.a sort.o                      # → exlib.a
gcc exercise.c -L. exlib.a -o exercise1
./exercise1

# ---- Dynamic（Exercise 2 流程，p.21~23 / p.27）----
gcc -c -fPIC sort.c                        # 重新以 -fPIC 編譯（會覆蓋剛剛的 sort.o）
gcc sort.o -shared -o exlib.so
gcc exercise.c -L. exlib.so -o exercise2
./exercise2                                # → error while loading shared libraries
sudo cp exlib.so /lib/exlib.so
./exercise2
ls -l                                      # 比較 exercise1 / exercise2 大小
```

### ✅ 已驗證
```
$ ./exercise1
Before sort: 61 67 88 54 4 30 36 8 84 71
After sort: 88 84 71 67 61 54 36 30 8 4
$ ./exercise2
./exercise2: error while loading shared libraries: exlib.so: cannot open shared object file: No such file or directory
$ LD_LIBRARY_PATH=. ./exercise2          # （驗證時代替 sudo cp 到 /lib）
Before sort: 56 8 97 23 3 49 26 61 67 57
After sort: 97 67 61 57 56 49 26 23 8 3
$ ls -l
-rw-r--r-- 1 root root   680 exercise.c
-rwxr-xr-x 1 root root 16232 exercise1      ← static 較大
-rwxr-xr-x 1 root root 16200 exercise2      ← dynamic 較小
-rw-r--r-- 1 root root  1602 exlib.a
-rw-r--r-- 1 root root    33 exlib.h
-rwxr-xr-x 1 root root 15112 exlib.so
-rw-r--r-- 1 root root   346 sort.c
-rw-r--r-- 1 root root  1456 sort.o
```

### 容易踩的坑
- **只差幾十 bytes 不是做錯**：sort 函式只有幾十 bytes 的機器碼，且兩者都還是動態連結 libc，所以差距很小（簡報截圖也只差 24 bytes：16224 vs 16200）。被問時答：「static 把 sort 的程式碼複製進執行檔；dynamic 只記錄 sort 在 exlib.so 裡。」
- **`-fPIC` 要重新編譯**：Exercise 1 的 sort.o 沒有 `-fPIC`。在本機（Ubuntu 預設 PIE）沒加 `-fPIC` 也剛好能包成 .so，但**有全域變數時會報錯**（實測：`relocation R_X86_64_PC32 against symbol 'counter' can not be used when making a shared object; recompile with -fPIC`）。考試一律照簡報加 `-fPIC`。
- **`.a`／`.so` 要放在 `exercise.c` 後面**：實測 `gcc exlib.a exercise.c -o x` 和 `gcc exlib.so exercise.c -o x` 都會 `undefined reference to 'sort_desc'`。順序照簡報：`gcc exercise.c -L. exlib.a -o exercise1`。
- 「不重複」一定要做檢查（practice 的 ex1.c 第一版就漏了）。
- `rand() % 100 + 1` 才是 1~100；`rand() % 100` 是 0~99。
- 氣泡排序內層上界 `n - 1 - i`，寫成 `n - i` 會讀到 `arr[n]` 越界。
