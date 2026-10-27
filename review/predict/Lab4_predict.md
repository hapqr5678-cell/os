# Lab 4（Library）上機考猜題

> 依據：`Lab4.pptx`（28 頁，含講者備註與截圖）、`practice/Lab4_ex/` 參考答案、`review/Lab4_review.md`
> 驗證環境：Ubuntu 24.04、gcc 13.3.0、GNU ar。每題都實際用 gcc/ar 建置並執行過（✅ 已驗證）。
> **dynamic 的部分**：驗證時用 `LD_LIBRARY_PATH=.`（或模擬 /lib 的資料夾）代替 `sudo cp ... /lib`；**考試請照簡報寫 `sudo cp xxx.so /lib/xxx.so`**。

---

## 0. Lab 4 專屬出題邏輯（補充共通邏輯 1~4）

| # | 觀察 | 證據（頁碼） | 對出題的意義 |
|---|---|---|---|
| A | **函式內容只是載體，流程才是考點**。Exercise 的 sort 本身很簡單，真正評分的是 `gcc -c → ar rcs → gcc main.c -L. xxx.a` 與 `-fPIC → -shared → 連結 → cp /lib → 執行` 這兩串指令。 | p.11、p.15、p.21、p.27 | 換函式（降冪、max/min、average、reverse…）是最便宜的變形；**指令序列必背**，C 程式只要能編譯就好。 |
| B | **範例（p.5）是「多個 .c → 一個 library」，Exercise 卻只有一個 sort.o**。 | p.5 add.c+sub.c；p.15 只有 sort.o | 很自然的變形是讓 Exercise 也包**多個 .o**（像 add/sub）。`ar rcs lib.a a.o b.o c.o`、`gcc a.o b.o c.o -shared -o lib.so`。 |
| C | **Exercise 2 固定是「同一支程式改用 shared 編 + `ls -l` 比大小」**，所以 static/dynamic 一定成對出現。 | p.26、p.27 截圖：exercise1 16224 / exercise2 16200 | 考題幾乎必然「兩種都做 + `ls -l`」，demo 時要能**說出為什麼 static 比較大**（library 程式碼被複製進執行檔）。 |
| D | **簡報花三頁講「執行時找不到 .so」**（p.22 錯誤訊息 → p.23 cp 到 /lib → p.24 ldd）。 | p.22~24 | 助教很可能要你**現場重現錯誤再修好**，或用 `ldd` 證明你的程式真的用到 .so。 |
| E | **Definition 頁的優缺點**（p.7「更新 library 需重新編譯整支程式」；p.17「執行時才連結」；講者備註 p.8「不需要 lib.a 即可執行」）。 | p.7、p.8 備註、p.17、p.19 備註 | 可以用「**更新 library**」或「**刪除 library**」的小實驗來驗證觀念 → 對應出題邏輯 2「用 exercise 驗證觀念」。 |
| F | p.10 放了 `ar` 的完整 usage（含 `t - display contents`、`d - delete`）。 | p.10 截圖 | `ar t xxx.a` 列出內容算是「簡報有出現」，可能被要求展示 library 裡有哪些 .o。 |
| G | 檔名、執行檔名都被指定（`exercise.c`、`sort.c`、`exlib.a`、`exlib.so`、`exercise1`、`exercise2`）。 | p.14、p.15、p.26 | 考題會指定名稱 → **照抄題目的檔名**，demo 時助教會看。 |

**範圍界線（不會考）**：`-lname`／`libxxx.so` 命名規則、`-static` 連結 libc、`soname`、`ldconfig`、`dlopen`、`nm`、`LD_LIBRARY_PATH`（只是本文件驗證用）。

---

## 題目總覽（依可能性排序）

| # | 題目 | 可能性 | 主要考點 |
|---|---|---|---|
| 1 | 降冪排序 library（10 個不重複亂數）：static + dynamic + `ls -l` | **高** | 換函式內容、整套流程 |
| 2 | 統計函式庫 max / min / average（3 個 .c → 1 個 library） | **高** | 多個 .o 打包（p.5 範例風格） |
| 3 | 原題重現 + `ldd` 驗證 + 刪除 library 後各自還能不能跑 | **中高** | p.22~24 找不到 .so、static 獨立性 |
| 4 | 更新 library（v1 → v2）後觀察 static / dynamic 行為差異 | **中** | p.7 / p.17 優缺點 |
| 5 | 擴充投影片範例：四則運算 library（add/sub/mul/divide） | **中** | 直接改 p.5 範例 |
| 6 | 字串函式庫：reverse + to_upper | **低** | 換資料型別（char 陣列） |

---

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

---

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

---

## 題 3：原題重現 + `ldd` 驗證 + 刪除 library 實驗

### 題目
1. 完成 Exercise 1、Exercise 2（sort 由小到大、5 個不重複亂數，`exercise1` / `exercise2`）。
2. 第一次執行 `exercise2` 時要出現「找不到 shared library」的錯誤，並修正它。
3. 分別對 `exercise1`、`exercise2` 執行 `ldd`，說明差異。
4. 把 `exlib.a`、`exlib.so`（含 /lib 裡那份）移走，再執行兩個程式，說明結果。

輸出範例：
```
$ ./exercise1                 ← exlib.a 已移走，仍可執行
Before sort: 60 52 99 95 35
After sort: 35 52 60 95 99
$ ./exercise2                 ← /lib/exlib.so 移走後失敗
./exercise2: error while loading shared libraries: exlib.so: cannot open shared object file: No such file or directory
```

### 變形自哪裡／考點
- p.22（執行時找不到 library）、p.23（cp 到 /lib）、p.24（ldd）。
- p.8 講者備註：「執行檔獨立存在，**不需要 lib.a 即可執行**」；p.17：dynamic「execution time 再連結」。

### 可能性：中高
簡報用 3 頁講「找不到 .so → 修好 → ldd」，是本 lab 唯一的「錯誤處理」劇情，很適合現場要你展示＋口頭解釋；刪除 library 則是直接驗證 p.8 / p.17 的觀念（出題邏輯 2）。原題本身也可能直接重考。

### 參考答案
**exlib.h**
```c
int *sort(int *arr);
```
**sort.c**
```c
#include "exlib.h"

int *sort(int *arr) {
    for (int i = 0; i < 5; i++)
        for (int j = i + 1; j < 5; j++)
            if (arr[j] < arr[i]) {
                int t = arr[i]; arr[i] = arr[j]; arr[j] = t;
            }
    return arr;
}
```
**exercise.c**
```c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "exlib.h"

int main(void) {
    int arr[5], cnt = 0;
    srand(time(NULL));
    while (cnt < 5) {
        int x = rand() % 100 + 1, dup = 0;
        for (int k = 0; k < cnt; k++)
            if (arr[k] == x) { dup = 1; break; }
        if (!dup) arr[cnt++] = x;
    }
    printf("Before sort: ");
    for (int i = 0; i < 5; i++) printf("%d ", arr[i]);
    printf("\n");
    sort(arr);
    printf("After sort: ");
    for (int i = 0; i < 5; i++) printf("%d ", arr[i]);
    printf("\n");
    return 0;
}
```
**指令序列**
```bash
gcc -c sort.c
ar rcs exlib.a sort.o
gcc exercise.c -L. exlib.a -o exercise1
./exercise1

gcc -c -fPIC sort.c
gcc sort.o -shared -o exlib.so
gcc exercise.c -L. exlib.so -o exercise2
./exercise2                       # 錯誤：cannot open shared object file
ldd exercise2                     # exlib.so => not found
sudo cp exlib.so /lib/exlib.so
./exercise2                       # 成功
ldd exercise1                     # 沒有 exlib
ldd exercise2                     # exlib.so => /lib/exlib.so
ls -l

# 刪除（移走）library 實驗
mkdir backup
mv exlib.a exlib.so backup/
sudo mv /lib/exlib.so backup/exlib_sys.so
./exercise1                       # 正常
./exercise2                       # 失敗
# 還原
sudo cp backup/exlib_sys.so /lib/exlib.so
```

### ✅ 已驗證（`./syslib` 資料夾 + `LD_LIBRARY_PATH=./syslib` 模擬 /lib）
```
$ ls -l exercise1 exercise2
-rwxr-xr-x 1 root root 16224 exercise1        ← 與簡報 p.27 截圖完全相同
-rwxr-xr-x 1 root root 16200 exercise2        ← 與簡報 p.27 截圖完全相同
$ ldd exercise2                                 （還沒 cp 之前）
	linux-vdso.so.1 (...)
	exlib.so => not found
	libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (...)
$ ldd exercise1
	linux-vdso.so.1 (...)
	libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (...)
	/lib64/ld-linux-x86-64.so.2 (...)
$ ldd exercise2                                 （cp 之後）
	exlib.so => ./syslib/exlib.so (...)         ← 考場會是 /lib/exlib.so
$ mv exlib.a exlib.so backup/ ; mv syslib/exlib.so backup/exlib_sys.so
$ ./exercise1
Before sort: 60 52 99 95 35
After sort: 35 52 60 95 99
$ ./exercise2
./exercise2: error while loading shared libraries: exlib.so: cannot open shared object file: No such file or directory
```

### 解釋（demo 時要能講）
- **編譯成功但執行失敗**：`-L.` 只在**連結時期**告訴 linker 去目前目錄找；**執行時**是 dynamic linker（loader）在找，它只找 `/lib`、`/usr/lib` 等系統路徑（p.22），所以要 cp 到 `/lib`。
- **ldd 差異**：exercise1 的 sort 已在連結時複製進執行檔，所以 ldd 不會列出 exlib；exercise2 只記錄「sort 在 exlib.so 裡」，執行時才載入。
- **刪除實驗**：static 執行檔獨立存在（p.8 備註）；dynamic 執行檔一定要找得到 .so（p.17）。

### 容易踩的坑
- **只移走目前目錄的 exlib.so 沒有用**：程式實際載入的是 `/lib/exlib.so`，要移走那一份才會失敗。
- `sudo cp exlib.so /lib/exlib.so` 後**不用重新編譯** exercise2，直接再執行即可。
- 簡報 p.27 截圖寫的是 `-L..`（兩個點，筆誤）；因為後面直接給了 `exlib.so` 檔案路徑所以仍然能過。考試請寫 `-L.`。
- 實驗完記得還原，否則之後 demo 其他題會失敗。

---

## 題 4：更新 library 後觀察 static / dynamic 行為差異

### 題目
1. 先用 v1 的 `sort.c`（由小到大，排序前印 `[exlib v1] ascending`）建出 `exercise1`（static）和 `exercise2`（dynamic）。
2. 把 `sort.c` 改成 v2（由大到小，印 `[exlib v2] descending`），**只重建 `exlib.a`、`exlib.so`，不要重新編譯 exercise.c**。
3. 再執行兩個程式，觀察差異並說明原因；最後重新連結 `exercise1` 讓它也變成 v2。

輸出範例（更新 library 後、未重新連結）：
```
$ ./exercise1
Before sort: 34 66 59 54 69
[exlib v1] ascending
After sort: 34 54 59 66 69
$ ./exercise2
Before sort: 34 66 59 54 69
[exlib v2] descending
After sort: 69 66 59 54 34
```

### 變形自哪裡／考點
- p.7 static 缺點：「如果要更新 library，**需要重新編譯整支程式**」。
- p.17 / p.19 備註：dynamic 在 execution time 才由 loader 載入 → 換掉 .so 就生效。

### 可能性：中
是「用 exercise 驗證觀念」（出題邏輯 2）的最佳候選，但需要改兩次程式、步驟多，比較像加分題或口試追問。

### 參考答案
**exlib.h**、**exercise.c**：同題 3。

**sort.c（v1）**
```c
#include <stdio.h>
#include "exlib.h"

/* v1：由小到大 */
int *sort(int *arr) {
    printf("[exlib v1] ascending\n");
    for (int i = 0; i < 5; i++)
        for (int j = i + 1; j < 5; j++)
            if (arr[j] < arr[i]) {
                int t = arr[i]; arr[i] = arr[j]; arr[j] = t;
            }
    return arr;
}
```
**sort.c（v2）**：只改三處
```c
#include <stdio.h>
#include "exlib.h"

/* v2：改成由大到小 */
int *sort(int *arr) {
    printf("[exlib v2] descending\n");
    for (int i = 0; i < 5; i++)
        for (int j = i + 1; j < 5; j++)
            if (arr[j] > arr[i]) {
                int t = arr[i]; arr[i] = arr[j]; arr[j] = t;
            }
    return arr;
}
```
**指令序列**
```bash
# ---- v1 ----
gcc -c sort.c && ar rcs exlib.a sort.o
gcc exercise.c -L. exlib.a -o exercise1
gcc -c -fPIC sort.c && gcc sort.o -shared -o exlib.so
gcc exercise.c -L. exlib.so -o exercise2
sudo cp exlib.so /lib/exlib.so
./exercise1 ; ./exercise2                 # 都是 v1

# ---- 修改 sort.c 為 v2，只重建 library ----
vim sort.c
gcc -c sort.c && ar rcs exlib.a sort.o    # r = replace，覆蓋 archive 裡的 sort.o
gcc -c -fPIC sort.c && gcc sort.o -shared -o exlib.so
sudo cp exlib.so /lib/exlib.so            # ★ 一定要再 cp 一次
./exercise1                               # 仍是 v1（舊程式碼已複製在執行檔內）
./exercise2                               # 變成 v2（執行時載入新的 .so）

# ---- 重新連結 static ----
gcc exercise.c -L. exlib.a -o exercise1
./exercise1                               # 變成 v2
```

### ✅ 已驗證（`./syslib` 模擬 /lib）
```
## v1
$ ./exercise1
Before sort: 34 66 59 54 69
[exlib v1] ascending
After sort: 34 54 59 66 69
$ ./exercise2
[exlib v1] ascending
After sort: 34 54 59 66 69

## 改成 v2、重建 .a/.so，但「忘了再 cp 到 /lib」
$ ./exercise2
[exlib v1] ascending                     ← 仍是 v1！loader 讀的是舊的 /lib/exlib.so
After sort: 34 54 59 66 69

## cp 到 /lib 之後
$ ./exercise1
[exlib v1] ascending                     ← static 沒重新連結，仍是舊程式碼
After sort: 34 54 59 66 69
$ ./exercise2
[exlib v2] descending                    ← dynamic 立即生效
After sort: 69 66 59 54 34

## 重新連結 static
$ gcc exercise.c -L. exlib.a -o exercise1 && ./exercise1
[exlib v2] descending
After sort: 69 66 59 54 34
```

### 容易踩的坑
- **重建 exlib.so 後忘了再 `sudo cp` 到 /lib** → dynamic 版看起來「沒更新」（上面實測）。
- 更新 static 時要記得 **`ar rcs` 會 replace** 同名 .o，不必先刪 exlib.a。
- `sort.c` 用了 `printf` 就要 `#include <stdio.h>`，不然 implicit declaration 警告。
- 不要改函式**簽章**（參數/回傳型別），否則 exercise2 沿用舊的 exercise.c 會不相容；這題只改內容。
- 被問「哪個比較好」：dynamic 更新方便、省空間；但 p.17 缺點是**版本不一時有相依性問題**（例如別的程式還需要 v1）。

---

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

---

## 題 6：字串函式庫 reverse + to_upper

### 題目
撰寫 `reverse.c`（將字串反轉）與 `upper.c`（將小寫轉大寫），共用 `strlib.h`，打包成 `strlib.a` 與 `strlib.so`。`main.c` 讀入一行字串，依序印出原字串、反轉後、轉大寫後。分別編成 `str_static`、`str_dynamic`，`ls -l` 比較。

輸出範例：
```
Input a string: Hello OS Lab4
Original : Hello OS Lab4
Reverse  : 4baL SO olleH
Upper    : 4BAL SO OLLEH
```

### 變形自哪裡／考點
- Exercise 的「傳陣列位址、函式就地修改陣列」換成 **char 陣列**；流程同 p.11 / p.21。

### 可能性：低
簡報完全沒碰字串處理，題目重心會偏離 library；但「傳位址、原地修改」與 sort 同精神，仍有可能當成換皮題。

### 參考答案
**strlib.h**
```c
void reverse(char *s);
void to_upper(char *s);
```
**reverse.c**
```c
#include <string.h>
#include "strlib.h"

void reverse(char *s) {
    int i = 0, j = strlen(s) - 1;
    while (i < j) {
        char t = s[i]; s[i] = s[j]; s[j] = t;
        i++; j--;
    }
}
```
**upper.c**
```c
#include "strlib.h"

void to_upper(char *s) {
    for (int i = 0; s[i] != '\0'; i++)
        if (s[i] >= 'a' && s[i] <= 'z')
            s[i] = s[i] - 'a' + 'A';
}
```
**main.c**
```c
#include <stdio.h>
#include <string.h>
#include "strlib.h"

int main(void) {
    char str[100];
    printf("Input a string: ");
    fgets(str, sizeof(str), stdin);
    str[strcspn(str, "\n")] = '\0';      /* 去掉 fgets 讀進來的換行 */
    printf("Original : %s\n", str);
    reverse(str);
    printf("Reverse  : %s\n", str);
    to_upper(str);
    printf("Upper    : %s\n", str);
    return 0;
}
```
**指令序列**
```bash
gcc -c reverse.c upper.c
ar rcs strlib.a reverse.o upper.o
gcc main.c -L. strlib.a -o str_static
./str_static

gcc -c -fPIC reverse.c upper.c
gcc reverse.o upper.o -shared -o strlib.so
gcc main.c -L. strlib.so -o str_dynamic
sudo cp strlib.so /lib/strlib.so
./str_dynamic
ls -l
```

### ✅ 已驗證
```
$ echo "Hello OS Lab4" | ./str_static
Input a string: Original : Hello OS Lab4
Reverse  : 4baL SO olleH
Upper    : 4BAL SO OLLEH
$ echo "nchu 2026" | LD_LIBRARY_PATH=. ./str_dynamic
Input a string: Original : nchu 2026
Reverse  : 6202 uhcn
Upper    : 6202 UHCN
$ ls -l str_static str_dynamic strlib.a strlib.so
-rwxr-xr-x 1 root root 16208 str_dynamic
-rwxr-xr-x 1 root root 16312 str_static
-rw-r--r-- 1 root root  3042 strlib.a
-rwxr-xr-x 1 root root 15544 strlib.so
```

### 容易踩的坑
- `fgets` 會把 `\n` 一起讀進來，不去掉的話反轉後換行會跑到最前面。
- 用 `scanf("%s")` 只能讀到空白前，題目若有空白要用 `fgets`。
- 函式名不要叫 `toupper`（`<ctype.h>` 已有），改 `to_upper`。
- `strlen` 需要 `#include <string.h>`。

---

## 驗證總表

| 題 | static 執行檔 | dynamic 執行檔 | 差距 | 執行結果 |
|---|---|---|---|---|
| 1 | exercise1 16232 | exercise2 16200 | 32 B | ✅ 降冪正確、不重複 |
| 2 | stat_static 16304 | stat_dynamic 16208 | 96 B | ✅ max/min/avg 正確；ldd 看得到 mylib.so |
| 3 | exercise1 16224 | exercise2 16200 | 24 B（與簡報 p.27 相同） | ✅ 移走 library：static 正常、dynamic 報錯 |
| 4 | — | — | — | ✅ 更新後 static 仍 v1、dynamic 變 v2；忘了 cp 則 dynamic 仍 v1 |
| 5 | calc_static 16296 | calc_dynamic 16176 | 120 B | ✅ 四則運算、除 0 判斷 |
| 6 | str_static 16312 | str_dynamic 16208 | 104 B | ✅ 反轉、轉大寫 |

結論：**library 函式越多，static 執行檔多出的大小越多**（函式程式碼被複製進去），但都只差幾十～一百多 bytes，因為 libc 兩邊都是動態連結。

---

## 考前 5 分鐘速查

**Static（背這 3 行）**
```bash
gcc -c a.c b.c                     # → a.o b.o
ar rcs xxx.a a.o b.o               # r=replace c=create s=symbol index；archive 名在前
gcc main.c -L. xxx.a -o prog_static
```

**Dynamic（背這 5 行）**
```bash
gcc -c -fPIC a.c b.c               # -fPIC：position-independent code，一定要重編
gcc a.o b.o -shared -o xxx.so
gcc main.c -L. xxx.so -o prog_dynamic
sudo cp xxx.so /lib/xxx.so         # 不 cp → cannot open shared object file
./prog_dynamic
```

**驗證**
```bash
ls -l        # static 執行檔 > dynamic 執行檔（差幾十 bytes 正常）
ldd prog_dynamic   # xxx.so => /lib/xxx.so；沒 cp 時顯示 not found
ldd prog_static    # 看不到 xxx
ar t xxx.a         # 列出 archive 裡的 .o
```

**必背觀念（一句話版）**
- static：**連結時把 library 程式碼複製進執行檔** → 檔案大、獨立（刪 .a 仍可跑）、不用管相依、**更新要重新編譯**、多程式共用浪費空間。
- dynamic：**執行時由 loader 載入** → 檔案小、多程式共用省空間、**換 .so 即更新**、有版本相依問題、.so 不見就跑不起來。
- `-L.` 只影響**連結時期**；執行時 dynamic linker 只找 `/lib`、`/usr/lib`。

**最常見的錯**
1. `.a`／`.so` 寫在 `main.c` 前面 → `undefined reference`。
2. `ar rcs` 把 .o 寫在前面 → `file format not recognized`。
3. 重建 .so 後忘了再 `sudo cp` 到 /lib → 還在跑舊版。
4. 忘了 `-fPIC`（有全域變數時必炸）。
5. 「不重複亂數」要檢查；`rand() % 100 + 1` 是 1~100；`srand(time(NULL))` 只呼叫一次。
6. 平均／除法要 `(double)`；函式別叫 `div`、`toupper`。
7. 檔名照題目：`exercise.c`、`sort.c`、`exlib.h`、`exlib.a`、`exlib.so`、`exercise1`、`exercise2`。
