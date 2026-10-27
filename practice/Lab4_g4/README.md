# Lab4_g4

> 來源:review/predict/Lab4_predict.md

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

---
> 資料夾說明:`sort_v1.c`、`sort_v2.c` 分別是題目中的 v1 / v2 版本。實作時檔名都叫 `sort.c`,先用 v1 建好,再把內容換成 v2 重建 library。
