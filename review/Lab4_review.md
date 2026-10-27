# Lab 4 複習：Library（Static & Dynamic）

> 來源：Lab4.pptx（28 張投影片，含講者備註）＋ practice/Lab4_ex/（ex1.c、sort.c、exlib.h、exlib.a、exlib.so、ex1、ex_dy）

---

## 1. 這個 LAB 在學什麼

把函式打包成 **library** 給其他程式使用，分成兩種：**Static Library（.a，archive）**在連結時就把程式碼複製進執行檔；**Dynamic／Shared Library（.so）**只在執行檔裡記錄「需要哪個 .so」，執行時才由 loader 載入。學會用 `gcc -c`、`ar rcs` 做 static library，用 `-fPIC`、`-shared` 做 shared library，並處理「執行時找不到 .so」的問題（複製到 `/lib`）、用 `ldd` 檢查相依。

---

## 2. 核心觀念

### 2.1 Library 是什麼
- 把一些行為（behavior）實作成的集合，提供定義良好的介面，照規定呼叫即可。
- 例：不用自己寫就能呼叫 `printf()`；用高階語言包裝 system call。

### 2.2 Static vs Dynamic（必考比較表）
| 特性 | Static Library（`.a`, Windows `.lib`） | Dynamic/Shared Library（`.so`, `.dll`, `.dylib`） |
|---|---|---|
| 連結時機 | **編譯（連結）時**直接把程式碼複製進執行檔 | **執行時**才載入 |
| 執行檔大小 | 大（包含 library code） | 小（只含符號／參照） |
| 更新 library | 要**重新編譯**程式 | **替換 .so 檔**即可 |
| 執行效率 | 較快（少一層間接呼叫） | 稍慢（需動態載入、間接呼叫） |
| 共用性 | 無，每個程式各一份 | 多個程式可**共用記憶體中的同一份** |
| 優點 | 不用考慮相依性；執行檔可獨立存在 | 省硬碟、省記憶體 |
| 缺點 | 多程式用同一 library 時浪費空間；更新要重編 | 不同程式需要不同版本時有**相依性問題** |

### 2.3 建置流程
**Static：**
```
add.c ─compiler→ add.o ┐
sub.c ─compiler→ sub.o ┴─archiver(ar)→ lib.a ┐
main.c ─compiler→ main.o ────────────────────┴─linker→ Executable（已含 add/sub 的程式碼）
```
執行檔獨立存在，**執行時不需要 lib.a**（刪掉 lib.a 程式仍能跑）。

**Dynamic：**
```
add.c ─compiler(-fPIC)→ add.o ┐
sub.c ─compiler(-fPIC)→ sub.o ┴─linker(-shared)→ lib.so ┐
main.c ─compiler→ main.o ───────────────────────────────┴─linker→ Executable（只記錄 add/sub 在 lib.so）
執行時：Loader 把 lib.so 載入記憶體，多個執行檔可共用
```

### 2.4 重要名詞
- **archive**：static library 本質上就是把多個 `.o` 打包成一個檔案。
- **`ar rcs`**：`r` = replace（新增或覆蓋 archive 中的檔案）、`c` = create（archive 不存在就建立，不顯示警告）、`s` = 建立符號索引（symbol index，等同 `ranlib`），讓 linker 快速找到符號；少了 `s` 可能連結變慢或找不到符號。
- **`-fPIC`**：Position-Independent Code，**位置無關碼**。shared library 會被載入到每個 process 不同的位址，程式碼不能寫死絕對位址。
- **`-shared`**：讓 gcc 產生 shared object（.so）而不是執行檔。
- **`-L.`**：告訴 linker 去**目前目錄**找 library。
- **dynamic linker / loader**：執行時負責找到並載入 .so；預設只找 `/lib`、`/usr/lib` 等系統路徑（以及 `/etc/ld.so.conf` 設定的路徑），**不會找目前目錄**。
- **`ldd`**：列出程式使用到的 shared library 及其路徑。

---

## 3. 指令速查表

| 名稱 | 用途 | 重要參數 | 範例 |
|---|---|---|---|
| `gcc -c` | 只編譯成 `.o`，不連結 | 可一次多檔 | `gcc -c add.c sub.c` |
| `ar rcs` | 打包 static library | r=replace、c=create、s=index | `ar rcs lib.a add.o sub.o` |
| `ar t` | 列出 archive 內容 | — | `ar t lib.a` |
| `gcc main.c -L. lib.a -o x` | 連結 static library | `-L.` 目前目錄 | `gcc main.c -L. lib.a -o example_static` |
| `gcc -c -fPIC` | 編譯成位置無關的 .o | — | `gcc -c -fPIC add.c sub.c` |
| `gcc -shared` | 產生 .so | — | `gcc add.o sub.o -shared -o lib.so` |
| `gcc main.c -L. lib.so -o x` | 連結 shared library | — | `gcc main.c -L. lib.so -o example_dynamic` |
| `sudo cp lib.so /lib/` | 讓 loader 找得到 | — | `sudo cp lib.so /lib/lib.so` |
| `ldd` | 列出相依的 .so | — | `ldd example_dynamic` |
| `ls -l` | 比較檔案大小 | — | `ls -l` |
| （補充）`LD_LIBRARY_PATH` | 不用 sudo，暫時加搜尋路徑 | — | `LD_LIBRARY_PATH=. ./example_dynamic` |
| （補充）`-lname` | 連結 `libname.a/.so` | 檔名要以 `lib` 開頭 | `gcc main.c -L. -lexlib`（需 `libexlib.so`） |
| （補充）`nm` | 列出 .o/.a/.so 的符號 | `T` = 有定義的函式 | `nm exlib.a` |

---

## 4. 程式範例逐行解說

### 4.1 投影片範例：main.c / add.c / sub.c / lib.h
```c
// lib.h
int add(int arg);
int sub(int arg);
// add.c
#include "lib.h"
int add(int arg) { arg++; return arg; }
// sub.c
#include "lib.h"
int sub(int arg) { arg--; return arg; }
// main.c
#include <stdio.h>
#include "lib.h"
int main() {
    int n1 = add(5);
    int n2 = sub(5);
    printf("After add, n1 = %d\n", n1);
    printf("After sub, n2 = %d\n", n2);
    return 0;
}
```
輸出：
```
After add, n1 = 6
After sub, n2 = 4
```
- `"lib.h"`（雙引號）會先在目前目錄找標頭；`<stdio.h>`（角括號）只找系統路徑。
- header 只放**宣告**，實作在 .c，打包進 library。

### 4.2 Static 流程實錄
```
$ gcc -c add.c sub.c          → 產生 add.o sub.o
$ ar rcs lib.a add.o sub.o    → 產生 lib.a
$ gcc main.c -L. lib.a -o example_static
$ ./example_static
After add, n1 = 6
After sub, n2 = 4
```

### 4.3 Dynamic 流程實錄（含錯誤）
```
$ gcc -c -fPIC add.c sub.c
$ gcc add.o sub.o -shared -o lib.so
$ gcc main.c -L. lib.so -o example_dynamic
$ ./example_dynamic
./example_dynamic: error while loading shared libraries: lib.so: cannot open shared object file: No such file or directory
$ sudo cp lib.so /lib/lib.so
$ ./example_dynamic
After add, n1 = 6
After sub, n2 = 4
$ ldd example_dynamic
    linux-vdso.so.1 (...)
    lib.so => /lib/lib.so (...)
    libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (...)
    /lib64/ld-linux-x86-64.so.2 (...)
```
**為什麼編譯成功、執行卻失敗？** 編譯時 `-L.` 告訴 linker 去目前目錄找；但**執行時**是 dynamic linker 在找，它只看系統路徑（`/lib`、`/usr/lib`），而 lib.so 不在清單中。複製到 `/lib` 就解決。

### 4.4 學生作業 practice/Lab4_ex

**exlib.h**
```c
int *sort(int *arr);
```
**sort.c**
```c
#include <stdio.h>
int *sort(int *arr) {
  for(int i = 0; i < 5; i++)
    for(int j = i; j < 5; j++)
      if(arr[j] < arr[i]) {          // 後面有更小的就交換到位置 i
        int temp = arr[j]; arr[j] = arr[i]; arr[i] = temp;
      }
  return arr;
}
```
這是類似 selection sort 的交換排序：外層每輪結束後 `arr[i]` 是剩下元素的最小值，結果由小到大。`j` 從 `i` 開始（自己跟自己比不會交換，無害）。

**ex1.c**
```c
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "exlib.h"
int main() {
  srand(time(NULL));               // 以目前時間當亂數種子
  int arr[5];
  printf("Before: ");
  for(int i = 0; i < 5; i++) {
    arr[i] = rand() % 102;         // 0~101
    printf("%d ", arr[i]);
  }
  sort(arr);                       // 陣列名稱就是位址，原地排序
  printf("\nAfter: ");
  for(int i = 0; i < 5; i++) printf("%d ", arr[i]);
  printf("\n");
}
```
範例輸出（亂數，每次不同）：
```
Before: 33 1 5 37 69
After: 1 5 33 37 69
```

**問題與建議：**
1. **沒有保證「不重複」**：題目要求「5 個**不重複**隨機整數」，`rand() % 102` 可能抽到相同數字。應在產生時檢查是否已出現（見下方參考解）。
2. **陣列長度寫死 5**：`sort(int *arr)` 只能排 5 個。可加長度參數 `sort(int *arr, int n)`（題目只要求接受陣列位址，所以不算錯，但較不通用）。
3. `sort.c` 裡 `#include <stdio.h>` 沒用到，可刪。
4. 題目要求檔名是 `exercise.c`、輸出檔 `exercise1`；學生用 `ex1.c`／`ex1`、`ex_dy`，demo 時注意名稱。
5. 從檔案大小看：`ex1` 16224 bytes、`ex_dy` 16192 bytes——static 版只大一點點，因為 sort 函式很小（投影片的 exercise1 16224 vs exercise2 16200 也一樣）。**真正的大小差異主要來自 libc 是否 static 連結**；兩者都還是動態連結 libc（補充）。

**參考解法：exercise.c（不重複）**
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
    sort(arr);
    printf("\nAfter sort: ");
    for (int i = 0; i < 5; i++) printf("%d ", arr[i]);
    printf("\n");
    return 0;
}
```

---

## 5. 上機考實作步驟

### Exercise 1：Static library
```bash
mkdir -p ~/lab4/exercise && cd ~/lab4/exercise
vim exlib.h          # int *sort(int *arr);
vim sort.c
vim exercise.c

gcc -c sort.c                         # → sort.o
ar rcs exlib.a sort.o                 # 把 sort.o 加入 exlib.a
gcc exercise.c -L. exlib.a -o exercise1
./exercise1
```

### Exercise 2：Shared library
```bash
gcc -c -fPIC sort.c                   # 重新以 -fPIC 編譯
gcc sort.o -shared -o exlib.so
gcc exercise.c -L. exlib.so -o exercise2
./exercise2                           # 若出現 cannot open shared object file ↓
sudo cp exlib.so /lib/exlib.so
./exercise2
ldd exercise2                         # 確認 exlib.so => /lib/exlib.so
ls -l                                 # 比較 exercise1 與 exercise2 大小
```

### 驗證 static 不需要 .a、dynamic 需要 .so（補充實驗）
```bash
mv exlib.a /tmp/ && ./exercise1       # 仍可執行
sudo mv /lib/exlib.so /tmp/ && ./exercise2   # 執行失敗
sudo mv /tmp/exlib.so /lib/           # 還原
```

---

## 6. 常見錯誤與陷阱

1. **做 .so 時沒加 `-fPIC`**：在 x86-64 上連結 `-shared` 會報 `relocation ... can not be used when making a shared object; recompile with -fPIC`。注意 Exercise 1 的 `sort.o` 沒用 `-fPIC`，Exercise 2 要**重新編譯**。
2. **執行時找不到 .so**：`error while loading shared libraries`。解法：複製到 `/lib`（投影片做法），或 `LD_LIBRARY_PATH=.`（補充）。
3. 以為 `-L.` 會影響執行時期搜尋 → 不會，`-L` 只影響**連結時期**。
4. `ar rc` 忘了 `s` → 可能沒有符號索引。
5. `ar` 的參數順序：**先放 archive 名稱，再放 .o**：`ar rcs exlib.a sort.o`。
6. `-lexlib` 要求檔名是 `libexlib.a/.so`；投影片是直接寫檔名 `exlib.a`，兩種寫法不要混。
7. 更新 static library 後忘了重新連結程式 → 執行檔仍是舊程式碼。
8. 沒 `#include "exlib.h"` → implicit declaration 警告（C99 後是錯誤）。
9. `srand()` 放在迴圈裡或沒呼叫 → 每次跑出一樣的數字。

---

## 7. 筆試模擬題

**Q1（選擇）** 關於 static library，何者錯誤？
(A) 副檔名常為 .a　(B) 在連結時期複製進執行檔　(C) 更新 library 後不需重新編譯程式　(D) 執行檔不需要 .a 就能執行
> **答：C**。static library 更新後必須重新連結程式。

**Q2（選擇）** `gcc -c -fPIC add.c` 中 `-fPIC` 的意義？
(A) 產生執行檔　(B) 產生位置無關碼　(C) 開啟最佳化　(D) 連結 pthread
> **答：B**。shared library 會被載入不同位址，需要 position-independent code。

**Q3（簡答）** `ar rcs lib.a add.o sub.o` 中 r、c、s 各代表什麼？
> **答：** r：取代或新增檔案到 archive；c：archive 不存在就建立且不警告；s：建立符號索引（等同 ranlib）。

**Q4（情境）** 執行 `./example_dynamic` 出現 `cannot open shared object file: No such file or directory`，但編譯時成功，為什麼？如何解決？
> **答：** 編譯時 `-L.` 只讓 linker 找到 lib.so；執行時的 dynamic linker 只搜尋系統路徑（如 /lib、/usr/lib），找不到目前目錄的 lib.so。解法：`sudo cp lib.so /lib/`（或設定 LD_LIBRARY_PATH）。

**Q5（選擇）** 想知道一個執行檔用到哪些 shared library，應用？
(A) `ar t`　(B) `ldd`　(C) `nm`　(D) `strace -c`
> **答：B**。

**Q6（是非）** 多個程式使用同一個 shared library 時，記憶體中可只載入一份程式碼。
> **答：對**。這是 shared library 省記憶體的原因。

**Q7（是非）** shared library 的執行效率通常比 static library 高。
> **答：錯**。static 較快（少一層間接呼叫）；shared 需動態載入。

**Q8（輸出預測）** 投影片 main.c 中 `add(5)`、`sub(5)` 的輸出？
> **答：** `After add, n1 = 6`、`After sub, n2 = 4`。

**Q9（排序題）** 將建立並使用 shared library 的指令依序排列：
① `gcc main.c -L. lib.so -o app`　② `gcc add.o sub.o -shared -o lib.so`　③ `gcc -c -fPIC add.c sub.c`　④ `sudo cp lib.so /lib/`　⑤ `./app`
> **答：** ③ → ② → ① → ④ → ⑤（④ 只要在 ⑤ 之前即可）。

**Q10（簡答）** 列出 dynamic library 的一個優點與一個缺點。
> **答：** 優點：省硬碟與記憶體、更新 library 只需替換 .so。缺點：不同程式需要不同版本時有相依性問題；執行時需載入，稍慢；執行環境必須有該 .so。

**Q11（選擇）** 刪除 `lib.a` 後執行 `./example_static` 會？
(A) 正常執行　(B) 找不到 library 錯誤　(C) Segmentation fault　(D) 需重新編譯才能執行
> **答：A**。程式碼已在連結時複製進執行檔。

**Q12（簡答）** 為何 Exercise 2 的 sort.c 需要重新編譯，不能沿用 Exercise 1 的 sort.o？
> **答：** Exercise 1 的 sort.o 沒有使用 `-fPIC`，不是位置無關碼，無法用 `-shared` 包成 .so（x86-64 上會報 relocation 錯誤）。
