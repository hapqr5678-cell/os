# Lab 5 複習：Kernel Module

> 來源：Lab5.pptx（36 張投影片，含講者備註）＋ practice/Lab5_ex/（d2ts.c、myage.c、Makefile）
> 註：投影片中的資料夾名稱寫 `lab3/...`（沿用舊版），內容就是本 Lab 的 kernel module。

---

## 1. 這個 LAB 在學什麼

寫可以在**執行中的 kernel 動態載入／卸載**的程式——**Loadable Kernel Module（.ko）**。學會 module 的基本結構（`init_module` / `cleanup_module`、`printk`、`MODULE_LICENSE`）、用 `obj-m` 的 Makefile 編譯、`insmod / lsmod / modinfo / dmesg / rmmod` 操作；用 `module_param` 從 `insmod` 傳參數；用 `EXPORT_SYMBOL` 讓一個 module 的變數／函式給另一個 module 用（載入／卸載順序有相依性）。Exercise：`date2timestamp` + `myage` 兩個 module 算出你活了幾年幾天。

---

## 2. 核心觀念

### 2.1 Package vs Kernel module
| | Package（如 apt、gcc、make） | Kernel module |
|---|---|---|
| 執行位置 | **user space** | **kernel space** |
| 如何用系統資源 | 呼叫 system call | 本身就在 kernel 裡，直接用 kernel 函式 |
| 載入方式 | 執行程式 | `insmod` 動態載入到執行中的 kernel |
| 典型例子 | 應用程式 | **驅動程式（driver）**、檔案系統 |

### 2.2 Kernel module vs User program（補充整理，常考）
| | User program | Kernel module |
|---|---|---|
| 進入點 | `main()` | `init_module()`（載入時）／`cleanup_module()`（卸載時） |
| 執行方式 | 從頭跑到尾結束 | 載入後常駐 kernel，被動等待被呼叫，直到 `rmmod` |
| 輸出 | `printf` → terminal | `printk` → kernel log（`dmesg` 看） |
| 可用函式庫 | libc（stdio.h…） | 只能用 kernel 提供的函式（`<linux/...>`） |
| 出錯後果 | 該 process 死掉 | 可能讓**整個系統**當掉（kernel panic／oops） |
| 權限 | user mode | kernel mode（最高權限），需要 sudo 才能載入 |

### 2.3 Module vs Lab2 的 syscall（`obj-m` vs `obj-y`）
- Lab2：`obj-y` → 編進 kernel 映像，要**重編整個 kernel + reboot**。
- Lab5：`obj-m` → 編成 `.ko`，**不用重編 kernel、不用重開機**，`insmod` 就載入，`rmmod` 就移除。保持 kernel 精簡、彈性。

### 2.4 Module 基本元素
- `#include <linux/module.h>`：所有 module 都需要。
- `int init_module(void)`：`insmod` 時執行。**回傳 0 = 成功；回傳非 0（負的錯誤碼）= 失敗，module 不會被載入。**
- `void cleanup_module(void)`：`rmmod` 時執行。
- `printk(KERN_INFO "...")`：`KERN_INFO` 是 log 等級（一般資訊）；還有 `KERN_ERR`（錯誤）、`KERN_WARNING`（警告）等。注意 `KERN_INFO` 和字串之間**沒有逗號**。
- `MODULE_LICENSE("GPL")`：宣告授權，表示原始碼開放、須遵守 GPL。（補充：沒寫會讓 kernel 被標為 tainted，且無法使用只 export 給 GPL 的符號。）
- `MODULE_AUTHOR`、`MODULE_DESCRIPTION`：資訊，會出現在 `modinfo`。

### 2.5 Module Parameters
- `module_param(name, type, perm)`：宣告參數；name=變數名、type=型別（如 `int`）、perm 本課設 0。
- `MODULE_PARM_DESC(name, desc)`：參數說明，會顯示在 `modinfo` 的 `parm:` 欄位。
- 使用：`sudo insmod hello_module_2.ko year=2024 month=8 date=1`。
- **陷阱（備註）**：輸入 `month=08` 這種前導 0 的寫法會被當成**八進位**；`01`~`07` 剛好正常，`08`、`09` 不是合法八進位 → `invalid param`。

### 2.6 Symbols 與 EXPORT_SYMBOL
- module 中宣告的全域變數、函式都是 **symbol**（`int year = 2023;` → variable symbol；`void doSomething(){}` → function symbol）。
- `EXPORT_SYMBOL(symbol_name)`：把 symbol 匯出到 kernel 符號表，**其他 module** 才能用 `extern` 宣告後使用。
- **相依順序**：B 用了 A 的 symbol →
  - 載入：**先 insmod A，再 insmod B**（否則 B 會 `Unknown symbol`）。
  - 卸載：**先 rmmod B，再 rmmod A**（A 的 `Used by` 計數不為 0 時無法移除，會顯示 `Module symbol_A is in use by: symbol_B`）。
- `lsmod` 的 `Used by` 欄位會顯示 `symbol_A ... 1 symbol_B`。

### 2.7 其他
- `cat /proc/modules`：列出已載入的 module；`lsmod` 其實就是解析這個檔案再輸出。
- `.ko` = Kernel Object，可在執行時載入而不必重編 kernel、重開機。
- apt 其他功能（備註）：`update`、`upgrade`、`remove`（保留設定檔）、`purge`（連設定檔刪）、`autoremove`、`search`、`show`、`list --installed`。
- 數已安裝套件：`cat /var/lib/dpkg/status | grep Package: > ~/Desktop/packages`，再 `wc -l`（`|` 是 pipe，`>` 是重新導向到檔案）。

---

## 3. 指令／API 速查表

| 名稱 | 用途 | 重要參數／說明 | 範例 |
|---|---|---|---|
| `make` | 依 Makefile 編譯出 `.ko` | 呼叫 kernel build 系統 | `make` |
| `make clean` | 清除編譯產物 | — | `make clean` |
| `modinfo x.ko` | 看 module 資訊 | license、author、description、parm | `modinfo hello_module.ko` |
| `sudo insmod x.ko [參數=值]` | 載入 module | **要加 `.ko`** | `sudo insmod hello_module_2.ko year=2024 month=8 date=1` |
| `lsmod` | 列出已載入 module | Size、Used by | `lsmod \| head -3` |
| `cat /proc/modules` | 同上（原始資料） | — | — |
| `sudo dmesg \| tail` | 看 kernel 訊息（最後 10 行） | `tail -2` 看最後 2 行 | `sudo dmesg \| tail -3` |
| `sudo rmmod name` | 卸載 module | **不加 `.ko`** | `sudo rmmod hello_module` |
| `printk(KERN_INFO "fmt", ...)` | 印到 kernel log | `KERN_ERR`、`KERN_WARNING` | `printk(KERN_INFO "ts=%lld\n", ts);` |
| `module_param(name, type, 0)` | 宣告參數 | — | `module_param(year, int, 0);` |
| `MODULE_PARM_DESC(name, "desc")` | 參數說明 | 顯示於 modinfo | `MODULE_PARM_DESC(year, "year of time");` |
| `EXPORT_SYMBOL(sym)` | 匯出符號 | 另一 module 用 `extern` | `EXPORT_SYMBOL(ts);` |
| `time64_t mktime64(year, mon, day, hour, min, sec)` | 日期轉 Unix timestamp（秒） | `#include <linux/time64.h>`；mon 1~12 | `mktime64(2001, 8, 1, 0, 0, 0)` |
| `ktime_get_real_seconds()` | 取得目前 Unix time（秒） | `#include <linux/timekeeping.h>`（補充：投影片沒指定用哪個函式） | `time64_t now = ktime_get_real_seconds();` |
| `return -EINVAL;` | init 失敗中止載入 | insmod 顯示 `Invalid parameters` | — |

---

## 4. 程式範例逐行解說

### 4.1 hello_module.c
```c
#include <linux/module.h>   // needed by all modules

int init_module(void) {
    printk(KERN_INFO "Hello module insert successfully!\n");
    return 0;               // 非 0 代表 init 失敗，module 無法載入
}

void cleanup_module(void) {
    printk(KERN_INFO "Hello module remove successfully!\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("William");
MODULE_DESCRIPTION("OSlab3: hello_module");
```
- `insmod` → dmesg 出現 `Hello module insert successfully!`
- `rmmod` → dmesg 出現 `Hello module remove successfully!`

### 4.2 Makefile（module 用）
```make
obj-m += hello_module.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```
- `obj-m += hello_module.o`：要編成 module 的目標，**名稱要和 .c 檔名相符**。
- `-C /lib/modules/$(uname -r)/build`：切換到**目前執行中 kernel** 的 build 目錄（kernel headers），用它的 build 系統。
- `M=$(PWD)`：module 原始碼在目前目錄。
- `modules`：編譯 module；`clean`：清除。
- 指令前**必須是 TAB**。

### 4.3 hello_module_2.c（module parameter）
```c
#include <linux/module.h>
int date = 1, month = 1, year = 2000;        // 預設值
module_param(date, int, 0);
MODULE_PARM_DESC(date, "date of time");
module_param(month, int, 0);
MODULE_PARM_DESC(month, "month of time");
module_param(year, int, 0);
MODULE_PARM_DESC(year, "year of time");

int init_module(void) {
    printk(KERN_INFO "Hello module 2 insert successfully!\n");
    printk(KERN_INFO "Today is %d/%d/%d.\n", year, month, date);
    return 0;
}
...
```
`sudo insmod hello_module_2.ko year=2024 month=8 date=1` → dmesg：
```
Hello module 2 insert successfully!
Today is 2024/8/1.
```
若不帶參數 → `Today is 2000/1/1.`（用預設值）。
`modinfo hello_module_2.ko` 會看到 `parm: date:date of time (int)` 等。
（dmesg 中出現的 `disagrees about version of symbol module_layout` 是版本不一致的警告，示範中仍載入成功。）

### 4.4 symbol_A.c / symbol_B.c
```c
/* symbol_A.c */
int symbol_variable = 99;
int symbol_function(int x){
    if (x > 0) return 1;
    else if (x == 0) return 0;
    else return -1;
}
int init_module(void){ printk(KERN_INFO "Symbol A insert successfully!\n"); return 0; }
void cleanup_module(void){ printk(KERN_INFO "Symbol A remove successfully!\n"); }
EXPORT_SYMBOL(symbol_variable);
EXPORT_SYMBOL(symbol_function);
MODULE_LICENSE("GPL");
```
```c
/* symbol_B.c */
extern int symbol_variable;
extern int symbol_function(int);
int init_module(void){
    printk(KERN_INFO "Symbol B insert successfully!\n");
    printk(KERN_INFO "Symbol_variable: %d\n", symbol_variable);
    printk(KERN_INFO "Symbol_function(100): %d\n", symbol_function(100));
    printk(KERN_INFO "Symbol_function(0): %d\n", symbol_function(0));
    printk(KERN_INFO "Symbol_function(-100): %d\n", symbol_function(-100));
    return 0;
}
```
Makefile 同時編兩個：`obj-m += symbol_A.o` 與 `obj-m += symbol_B.o`。
先 insmod A、再 insmod B，dmesg：
```
Symbol A insert successfully!
Symbol B insert successfully!
Symbol_variable: 99
Symbol_function(100): 1
Symbol_function(0): 0
Symbol_function(-100): -1
```
`lsmod` 顯示 `symbol_A ... 1 symbol_B`（被 B 使用）。（投影片的 symbol_B.c 中 `MODULE_DESCRIPTION` 寫成 `symbol_A`，是投影片的小筆誤。）

### 4.5 學生作業 practice/Lab5_ex/d2ts.c（date2timestamp）
```c
int year = 2000, month = 1, date = 1;
time64_t ts = 0;
module_param(year, int, 0);  MODULE_PARM_DESC(year, "birth year");
module_param(month, int, 0); MODULE_PARM_DESC(month, "birth month");
module_param(date, int, 0);  MODULE_PARM_DESC(date, "birth date");
EXPORT_SYMBOL(ts);                       // 匯出給 myage 用

int is_adult(time64_t now) {             // 額外功能：是否滿 18 歲
    time64_t adult_ts = mktime64(year + 18, month, date, 0, 0, 0);
    return now >= adult_ts;
}
EXPORT_SYMBOL(is_adult);

int init_module(void) {
    int days_in_month[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    int leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    if (leap) days_in_month[1] = 29;     // 閏年 2 月 29 天
    if (year < 1970 || month < 1 || month > 12 ||
        date < 1 || date > days_in_month[month - 1]) {
        printk(KERN_ERR "Invalid argument(s).\n");
        return -EINVAL;                  // 負值 → insmod 失敗
    }
    ts = mktime64(year, month, date, 0, 0, 0);
    printk(KERN_INFO "date2timestamp insert successfully! ts=%lld\n", ts);
    return 0;
}
```
- 驗證順序正確：先確認 `month` 在 1~12，**才**用 `days_in_month[month - 1]`（`||` 短路求值，避免越界）。
- 閏年判斷：可被 4 整除且不被 100 整除，或可被 400 整除。
- `year=2001 month=8 date=1` → `ts=996624000`（2001-08-01 00:00:00 UTC 的 Unix 時間）。
- `year=1970 month=13 date=-1` → `Invalid argument(s).`，insmod 顯示 `ERROR: could not insert module ...: Invalid parameters`。

**practice/Lab5_ex/myage.c**
```c
extern time64_t ts;
extern int is_adult(time64_t now);
int init_module(void) {
    time64_t now = ktime_get_real_seconds();   // 目前時間（秒）
    time64_t diff = now - ts;
    long long days, years;
    printk(KERN_INFO "myage insert successfully!\n");
    if (diff < 0) {
        printk(KERN_WARNING "Your birthday is in the future! (diff=%lld)\n", diff);
        return 0;
    }
    days  = diff / 86400;      // 1 天 = 86400 秒
    years = days / 365;
    days  = days % 365;
    printk(KERN_INFO "Wow you have survived on this world %lld years and %lld days!\n", years, days);
    ...
}
```

**評論與注意事項：**
1. 整體邏輯正確，符合題目（參數、export ts、無效日期回 `-EINVAL`、myage 相減並處理 <0 情況）。
2. **module 名稱**：Makefile 是 `obj-m += d2ts.o`，產生的是 `d2ts.ko`；題目指定的名稱是 `date2timestamp`。demo 時若助教要看 `date2timestamp.ko`，請把檔名改成 `date2timestamp.c` 並改 Makefile 的 `obj-m`。
3. **年數用 365 天近似**：沒考慮閏年，活很多年時「天數」會差幾天（題目只要求轉成年跟日，可接受，但要知道是近似）。
4. **`year < 1970` 被當成無效**：題目沒有這樣要求；`mktime64` 其實能處理 1970 前（得到負的 ts）。若助教輸入 1969 年生日會被拒絕——說得出理由即可。
5. myage 沒有 `#include <linux/time64.h>`（`time64_t` 透過其他標頭間接取得，能編譯）。
6. **相依順序**：必須先 `insmod d2ts.ko` 再 `insmod myage.ko`；卸載時先 `rmmod myage` 再 `rmmod d2ts`。
7. 投影片範例輸出中助教的 `ts=63163929600`（2001/8/1）並不是 Unix timestamp（`mktime64` 應得到 996624000），推測助教範例用了其他算法；**以題目提示的 `mktime64` 為準**（「其餘文字不須照打，看得懂即可」）。

---

## 5. 上機考實作步驟

```bash
# ===== hello_module =====
mkdir -p ~/lab5/hello_module && cd ~/lab5/hello_module
vim hello_module.c
vim Makefile             # obj-m += hello_module.o ；指令前用 TAB
make                     # 產生 hello_module.ko
modinfo hello_module.ko
sudo insmod hello_module.ko
lsmod | grep hello
sudo dmesg | tail
sudo rmmod hello_module  # 不加 .ko
sudo dmesg | tail -2

# ===== module parameter =====
cd ~/lab5/hello_module_2
make
modinfo hello_module_2.ko
sudo insmod hello_module_2.ko year=2024 month=8 date=1
sudo dmesg | tail -3
sudo rmmod hello_module_2

# ===== symbol =====
cd ~/lab5/symbol
make                     # Makefile 有兩行 obj-m
sudo insmod symbol_A.ko  # 先 A
sudo insmod symbol_B.ko  # 再 B
lsmod | head -3
sudo dmesg | tail
sudo rmmod symbol_B      # 先卸 B
sudo rmmod symbol_A      # 再卸 A

# ===== Exercise =====
cd ~/lab5/exercise       # date2timestamp.c、myage.c、Makefile(兩行 obj-m)
make
sudo insmod date2timestamp.ko year=1970 month=13 date=-1   # 應失敗：Invalid parameters
sudo dmesg | tail -2                                       # Invalid argument(s).
sudo insmod date2timestamp.ko year=2001 month=8 date=1
sudo dmesg | tail -2                                       # 印出 ts
sudo insmod myage.ko
sudo dmesg | tail -2                                       # 活了 X years and Y days
sudo rmmod myage
sudo rmmod date2timestamp
make clean
```

### Exercise 參考解法（精簡、符合題目「寫在 init_module 內」）
```c
/* date2timestamp.c */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/time64.h>

static int year = 2000, month = 1, date = 1;
module_param(year, int, 0);  MODULE_PARM_DESC(year, "birth year");
module_param(month, int, 0); MODULE_PARM_DESC(month, "birth month");
module_param(date, int, 0);  MODULE_PARM_DESC(date, "birth date");

time64_t ts;
EXPORT_SYMBOL(ts);

int init_module(void) {
    int mdays[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) mdays[1] = 29;
    if (month < 1 || month > 12 || date < 1 || date > mdays[month - 1]) {
        printk(KERN_ERR "Invalid argument(s).\n");
        return -EINVAL;
    }
    ts = mktime64(year, month, date, 0, 0, 0);
    printk(KERN_INFO "date2timestamp insert successfully! ts=%lld\n", ts);
    return 0;
}
void cleanup_module(void) { printk(KERN_INFO "date2timestamp removed\n"); }
MODULE_LICENSE("GPL");
```
```c
/* myage.c */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/timekeeping.h>
extern time64_t ts;

int init_module(void) {
    time64_t diff = ktime_get_real_seconds() - ts;
    printk(KERN_INFO "myage insert successfully!\n");
    if (diff < 0) {
        printk(KERN_INFO "You are not born yet!\n");
        return 0;
    }
    printk(KERN_INFO "Wow you have survived on this world %lld years and %lld days!\n",
           diff / 86400 / 365, (diff / 86400) % 365);
    return 0;
}
void cleanup_module(void) { printk(KERN_INFO "myage removed\n"); }
MODULE_LICENSE("GPL");
```
```make
obj-m += date2timestamp.o
obj-m += myage.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```

---

## 6. 常見錯誤與陷阱

1. `rmmod hello_module.ko` → 應該是 `rmmod hello_module`（**rmmod 不加 .ko，insmod 要加 .ko**）。
2. Makefile 指令前用空白 → `missing separator`。
3. `obj-m` 的名稱和 .c 檔名不一致 → 找不到來源。
4. 忘了 `sudo` → `Operation not permitted`。
5. 以為 printk 會印到 terminal → 要 `dmesg`。
6. `printk(KERN_INFO, "...")` 多了逗號 → `KERN_INFO` 是字串前綴，要直接相接。
7. `init_module` 回傳非 0 → 載入失敗（這也是 Exercise 中止安裝的方法，要回傳**負**的錯誤碼如 `-EINVAL`）。
8. 載入順序錯（先載 B）→ `Unknown symbol in module`；卸載順序錯（先卸 A）→ `Module is in use`。
9. 參數 `month=08` → 八進位問題導致 invalid param；請輸入 `month=8`。
10. 忘了 `EXPORT_SYMBOL` 或另一邊沒寫 `extern` → 連結不到。
11. 在 kernel module 中用 `printf`、`<stdio.h>`、浮點數運算 → 不行（kernel 不建議使用浮點數，補充）。
12. 沒裝 kernel headers（`/lib/modules/$(uname -r)/build` 不存在）→ 補充：`sudo apt install linux-headers-$(uname -r)`。

---

## 7. 筆試模擬題

**Q1（選擇）** 下列何者最適合做成 kernel module？
(A) 文字編輯器　(B) 新硬體的驅動程式　(C) 網頁瀏覽器　(D) gcc
> **答：B**。module 用來擴充 kernel 功能，最常見是驅動程式。

**Q2（是非）** 載入 kernel module 需要重新編譯 kernel 並重開機。
> **答：錯**。這是 module 相對於 `obj-y` 編進 kernel 的優點：可動態載入／卸載。

**Q3（選擇）** 卸載 module 的正確指令？
(A) `sudo rmmod hello.ko`　(B) `sudo rmmod hello`　(C) `sudo insmod -r hello`　(D) `sudo modinfo -d hello`
> **答：B**。

**Q4（簡答）** `init_module()` 回傳非 0 會發生什麼事？Exercise 中如何利用？
> **答：** 代表初始化失敗，module 不會被載入，insmod 回報錯誤。Exercise 遇到無效日期時 `printk(KERN_ERR ...)` 並 `return -EINVAL;` 中止安裝。

**Q5（輸出預測）** symbol_B 的 `symbol_function(-100)` 在 dmesg 顯示？
> **答：** `Symbol_function(-100): -1`。

**Q6（順序）** module B 使用 module A export 的 symbol，寫出正確的載入與卸載順序。
> **答：** 載入：insmod A → insmod B；卸載：rmmod B → rmmod A。

**Q7（選擇）** `MODULE_PARM_DESC` 的內容會出現在哪個指令的輸出？
(A) `lsmod`　(B) `dmesg`　(C) `modinfo`　(D) `insmod`
> **答：C**。

**Q8（簡答）** 解釋 Makefile 中 `make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules`。
> **答：** `-C` 切到目前執行中 kernel 的 build 目錄，使用它的 kernel build 系統；`M=$(PWD)` 指定 module 原始碼在目前目錄；`modules` 是要建置的目標（編譯 module）。

**Q9（輸出預測）** 對 hello_module_2 執行 `sudo insmod hello_module_2.ko year=2025`（未給 month、date），dmesg 第二行？
> **答：** `Today is 2025/1/1.`（未給的參數用預設值 1）。

**Q10（是非）** `lsmod` 的資料來源是 `/proc/modules`。
> **答：對**。lsmod 解析 /proc/modules 並格式化輸出。

**Q11（簡答）** 為什麼輸入 `month=09` 會失敗？
> **答：** 前導 0 被解析為八進位，09 不是合法八進位數字，造成 invalid param。應輸入 `month=9`。

**Q12（選擇）** 關於 `MODULE_LICENSE("GPL")` 何者正確？
(A) 讓 module 執行更快　(B) 宣告 module 以 GPL 授權、原始碼開放　(C) 讓 module 可以使用 printf　(D) 指定 module 作者
> **答：B**。

**Q13（簡答）** 比較 kernel module 與一般 user program 的兩個差異。
> **答：** ① 進入點：user program 是 `main`，module 是 `init_module`／`cleanup_module`；② 輸出：`printf` 到 terminal vs `printk` 到 kernel log；③ module 在 kernel space 執行，出錯可能讓整個系統當掉；④ module 不能用 libc。（任兩點）
