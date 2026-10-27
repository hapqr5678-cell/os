# Lab4_g3

> 來源:review/predict/Lab4_predict.md

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
