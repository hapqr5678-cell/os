# Lab 2 上機考猜題:Adding a New System Call

> 依據:`Lab2.pptx`(45 張,含講者備註與投影片截圖)、`review/Lab2_review.md`。
> 頁碼 = 投影片編號(p.13 = 第 13 張)。
> ⚠️ **kernel 端程式無法在此環境驗證**:這裡沒有 v7.1.3 原始碼,也不能開機測試。我只做了人工語法檢查(對照 `SYSCALL_DEFINEn` 巨集規則、`pr_info`/`printk` 用法、標頭檔)。
> ✅ **user 端 test.c 都已驗證**:全部用 `gcc -Wall -Wextra -fsyntax-only` 檢查過,沒有錯誤也沒有警告,也實際編譯執行過。檔案在 `/tmp/claude-0/-home-user-os/1dcd3112-7322-5920-934d-1535c60cff9f/scratchpad/lab2/`。

---

## 0. 投影片圖片裡才有的關鍵內容(文字檔看不到)

| 頁 | 圖片內容 | 考試意義 |
|---|---|---|
| p.5 | `User Program → write()` → `C library(libc write())` → **System Call**(user/kernel 分界線) → `Kernel(sys_write())` | 觀念題:libc 只是包裝(wrapper),真正做事的是 kernel 的 `sys_xxx` |
| p.11 | Makefile 範例:`test: test.o` / `gcc test.o -o test`;`test.o: test.c` / `gcc -c test.c -o test.o`;`clean:` / `rm -f test test.o` | 可能要你自己寫 test 的 Makefile(command 前要用 **TAB**) |
| p.13 | `hello.c`:`#include <linux/kernel.h>`、`#include <linux/syscalls.h>`、`SYSCALL_DEFINE0(hello){ pr_info("Hello\n"); return 0; }` | 標準範本,考試直接照這個改 |
| p.15 | syscalls.h 第 303 行 `#ifndef CONFIG_ARCH_HAS_SYSCALL_WRAPPER` 的**下一行(304)**加 `asmlinkage long sys_hello(void)`(截圖少了分號,文字版有分號) | 加的位置、**分號要記得加** |
| p.16 | Kbuild 第 100 行 `obj-y += hello/`(接在 `obj-y += $(ARCH_DRIVERS)` 後面) | 加在檔尾 |
| p.18 | `syscall_64.tbl` 第 386 行 `462 common hello sys_hello`,上一行是 `461 common lsm_list_modules` | 這張截圖來自 **6.8 版** 的 medium 教學(見下方 ⚠️) |
| p.37 | `test.c`:`#include <stdio.h>`、`<sys/syscall.h>`、`<unistd.h>`;`printf("%ld\n", syscall(462));` | user 端範本 |
| p.38 | `make test` → `chmod +x test` → `./test` → 印 `0` | terminal 只會看到**回傳值** |
| p.39 | `sudo dmesg \| tail` → `[ 684.324225] Hello` | 輸出在 kernel log |
| p.42 | Exercise 2 的 dmesg:連續三行 `[ 554.66250x] hello`(**小寫** hello) | Ex2 預期輸出的格式 |

> ⚠️ **syscall 編號可能被占用(重要)**:p.18 的截圖是 6.8 版,表中最後一號是 461,所以 462 是空的。但較新的 kernel 已經把 462 以後的號碼分配出去了(依我的記憶,6.10 起 462 = `mseal`)。這個環境跑的是 6.18 kernel,我實測 `syscall(462,0,0,0)` 回傳 **0(這個 syscall 存在)**,`syscall(470)` 才回傳 ENOSYS。所以考場的 v7.1.3 原始碼裡,462 很可能已經被占用。
> **考場做法**:先跑 `grep -n "^46[0-9]" arch/x86/entry/syscalls/syscall_64.tbl` 看清楚。如果 462 已經有人用,就照**助教/投影片要求的號碼**;助教沒指定的話,改用 common 區最後一號 +1(不可放進 512~547 的 x32 區),並且 **test.c 裡的號碼也要一起改**。如果你的 VM 在 lab 時已經用 462 成功編過、測過,就沿用 462。

---

## 1. Lab 2 專屬出題邏輯(推敲)

### 1.1 先看考試的現實限制:重編 kernel 很花時間
- 第一次完整編譯要 10 分鐘以上(p.22),所以考試**幾乎不可能**要你從 `git clone` 開始做。比較可能的情況是:考場用你自己做過 lab 的 VM,**kernel 已經編過**、hello syscall 也已經在裡面。
- 只改 `hello/hello.c` 再 `make -j $(nproc)` 屬於**增量編譯**,只會重編 `hello.o` 並重新連結 vmlinux,通常幾分鐘內完成。接著 `modules_install` → `install` → `reboot`(p.43)。
- **只改 hello.c 最省時間**,所以考題的主力應該是「改 syscall 的實作內容」,也就是 Exercise 2 的路線。
- 會拖慢時間的修改:
  - 改 `include/linux/syscalls.h`:很多檔案都 include 它,改了時間戳記就會觸發**大量重編**。
  - 改 `syscall_64.tbl`:會重新產生 syscall 表頭檔,也要重編一部分檔案。
  - 所以「新增第二個 syscall」這類題目的可能性比較低,出了也比較可能出成**筆試或流程題**。

> 💡 **關於 syscalls.h 宣告**(⚠️ 無法在此環境驗證,依 kernel 原始碼結構推論):p.15 截圖顯示宣告放在 `#ifndef CONFIG_ARCH_HAS_SYSCALL_WRAPPER` 區塊裡。x86_64 有開這個選項,所以在 x86_64 上這段宣告其實**不會被編譯**;真正的進入點由 `SYSCALL_DEFINEn` 自動產生(`__x64_sys_hello`)。因此宣告的參數和 `SYSCALL_DEFINEn` 不一致時,x86_64 上**通常不會編譯失敗**。
> **建議**:照規矩還是要同步修改(助教可能檢查、筆試會考)。但如果考場時間很緊,而且題目沒要求改宣告,可以知道「不改也能編、能跑」。要不要改,**以助教要求為準**。

### 1.2 這個 lab 的出題公式
1. **變的是 syscall 做什麼**:Ex1 是 0 個參數、印一次;Ex2 是 1 個參數、印 n 次。合理的延伸是 **2 個參數(`SYSCALL_DEFINE2`)**,或者用**回傳值**把結果帶回 user space。
2. **要驗證的觀念**:
   - kernel 的輸出在 `dmesg`,不在 terminal;terminal 只看得到 `syscall()` 的回傳值(p.38、p.39)。
   - 參數從 user space 經由 `syscall(編號, 參數...)` 傳進 kernel(p.42)。
   - 改完一定要重編 → install → reboot 才會生效(p.43 備註)。
3. **固定指定的語法/API**:`SYSCALL_DEFINEn(name, 型別, 名稱, ...)`、`pr_info` / `printk(KERN_INFO ...)`、`syscall(編號, ...)`、`%ld`、`sudo dmesg | tail`。
4. **不會考簡報沒教的東西**:例如傳指標要用 `copy_from_user` / `copy_to_user`,簡報完全沒教,所以**不列入猜題**;參數只會是整數。
5. **筆試或口試可能問的**:要改哪 5 個檔案、各加哪一行、`obj-y` 的意義、`asmlinkage`、`common` 的意義、編譯安裝指令的順序、GRUB 怎麼選 `7.1.3-dirty`。

---

## 2. 猜題(依可能性排序)

### 題 1(可能性:**高**)Exercise 2 原題:傳入 n,印 n 次 hello

**題目**:改寫範例的 `hello.c`,用 `SYSCALL_DEFINE1()` 讓 syscall 接收一個參數 n,印出 n 次 hello。執行 `syscall(462, 3)`。

**預期輸出**
```
$ ./test
0
$ sudo dmesg | tail
[  554.662505] hello
[  554.662509] hello
[  554.662510] hello
```

- **變形來源**:p.42 Exercise-2 本身(40 分),以及 p.43 的重編流程。
- **為什麼可能性高**:Ex2 是唯一有實作內容的練習,上機考最直接的出法就是「現場重做 Ex2」或「只改 n 和印的字」。
- **考的觀念**:`SYSCALL_DEFINE1` 的寫法、參數傳遞、dmesg。

**參考答案**(⚠️ kernel 端無法在此環境驗證)

`~/linux/hello/hello.c`
```c
#include <linux/kernel.h>
#include <linux/syscalls.h>

SYSCALL_DEFINE1(hello, int, n)
{
    int i;

    for (i = 0; i < n; i++)
        pr_info("hello\n");      /* 等同 printk(KERN_INFO "hello\n"); */
    return 0;
}
```

其他檔案

| 檔案 | 內容 | 這題要不要動 |
|---|---|---|
| `~/linux/hello/Makefile` | `obj-y += hello.o` | 不用(lab 時已加好) |
| `~/linux/include/linux/syscalls.h` | `asmlinkage long sys_hello(int n);`(原本是 `(void)`) | 建議同步改(見 1.1 說明) |
| `~/linux/Kbuild` | `obj-y += hello/` | 不用 |
| `~/linux/arch/x86/entry/syscalls/syscall_64.tbl` | `462 common hello sys_hello` | 不用(編號、名稱不變) |

user 端 `test.c`(✅ 已通過 gcc 檢查)
```c
#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
int main(void) {
    printf("%ld\n", syscall(462, 3));
    return 0;
}
```

編譯與測試
```bash
cd ~/linux && make -j $(nproc) && sudo make -j $(nproc) modules_install && sudo make install && reboot
# GRUB 選 Linux 7.1.3-dirty
uname -r
gcc test.c -o test && ./test
sudo dmesg | tail
```

**容易踩的坑**
- 寫成 `SYSCALL_DEFINE1(hello, int n)`:**型別和名稱要用逗號分開**,正確寫法是 `hello, int, n`。
- 在 for 迴圈裡宣告 `for (int i = 0; ...)`:新版 kernel 用 gnu11,其實可以編過;但寫成「先宣告 `int i;`」最保險。
- `pr_info("hello")` 漏掉 `\n`:dmesg 可能會把好幾次輸出黏成同一行(會被當成 continuation)。
- 改完沒有 reboot,或 reboot 後沒在 GRUB 選新 kernel:跑到的還是舊的 syscall。先用 `uname -r` 確認。
- 以為 terminal 會印出 hello:terminal 只會印出 `0`。

---

### 題 2(可能性:**高**)SYSCALL_DEFINE2:兩個參數相加,並回傳結果

**題目**:改寫 hello syscall,用 `SYSCALL_DEFINE2()` 接收兩個整數 a、b。在 kernel log 印出 `a + b = 結果`,並把結果**回傳**給 user program 印出。執行 `syscall(462, 3, 5)`。

**預期輸出**
```
$ ./test
return = 8
$ sudo dmesg | tail -1
[  xxx.xxxxxx] 3 + 5 = 8
```

- **變形來源**:p.42 的 `SYSCALL_DEFINE1` 推廣成 `DEFINE2`,加上 p.38 備註「syscall() 成功回傳 0 或正數」(回傳值)、p.37 的 `printf("%ld\n", syscall(...))`。
- **為什麼可能性高**:符合共通出題邏輯「把核心機制換個小題目再用一次」,也就是「1 個參數 → 2 個參數」,跟其他 lab 的 add/sub 題型如出一轍。而且只要改 hello.c,時間來得及。
- **考的觀念**:多參數巨集「型別, 名稱」成對出現;回傳值會變成 user 端 `syscall()` 的回傳值。

**參考答案**(⚠️ kernel 端無法在此環境驗證)

`~/linux/hello/hello.c`
```c
#include <linux/kernel.h>
#include <linux/syscalls.h>

SYSCALL_DEFINE2(hello, int, a, int, b)
{
    int sum = a + b;

    pr_info("%d + %d = %d\n", a, b, sum);
    return sum;
}
```

其他檔案

| 檔案 | 內容 |
|---|---|
| `include/linux/syscalls.h` | `asmlinkage long sys_hello(int a, int b);` |
| `hello/Makefile`、`Kbuild`、`syscall_64.tbl` | 不用動 |

user 端 `test.c`(✅ 已通過 gcc 檢查)
```c
#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
int main(void) {
    long r = syscall(462, 3, 5);
    printf("return = %ld\n", r);
    return 0;
}
```

**容易踩的坑**
- 寫成 `SYSCALL_DEFINE2(hello, int a, int b)`:錯。數字 2 代表 2 組「型別, 名稱」,全部用逗號隔開。
- `DEFINEn` 的 n 要和參數組數一致。寫 `SYSCALL_DEFINE1(hello, int, a, int, b)` 會編譯錯誤。
- 如果改成減法而且結果是負數(例如 3−5 = −2):user 端會看到 **-1**,errno 被設成 2。原因是 libc 的 `syscall()` 把 -1~-4095 之間的回傳值當成錯誤碼。
  - 這是延伸觀念,簡報沒教,但跟 p.38「失敗回傳 -1」有關。
  - 如果題目要做減法,答案要從 dmesg 看,不要靠回傳值。
- user 端用 `%d` 印 `long`:會有警告,要用 `%ld`。

---

### 題 3(可能性:**中~高**)用回傳值把計算結果帶回 user space

**題目**:`syscall(462, n)` 要在 kernel 計算 1 + 2 + … + n,每一步都用 `printk` 印出目前累加值,最後把總和回傳。user 端印出回傳值。以 n = 5 為例。

**預期輸出**
```
$ ./test
sum(1..5) = 15
$ sudo dmesg | tail -5
[  xxx.xxxxxx] i=1 sum=1
[  xxx.xxxxxx] i=2 sum=3
[  xxx.xxxxxx] i=3 sum=6
[  xxx.xxxxxx] i=4 sum=10
[  xxx.xxxxxx] i=5 sum=15
```

- **變形來源**:p.42 Ex2(迴圈 n 次 + DEFINE1)、p.13 備註(`printk(KERN_INFO ...)` 是 `pr_info` 的原型)、p.38 備註(回傳值)。
- **可能性與理由**:結構跟 Ex2 幾乎一樣(迴圈跑 n 次),只多了「回傳值」這個驗證點,剛好可以檢驗學生懂不懂「terminal 的 0 是怎麼來的」。也可能換成 n!、印 n 次學號等小變化。
- **考的觀念**:`return` 的值就是 `syscall()` 的回傳值;`printk` 和 `pr_info` 是等價寫法。

**參考答案**(⚠️ kernel 端無法在此環境驗證)

`~/linux/hello/hello.c`
```c
#include <linux/kernel.h>
#include <linux/syscalls.h>

SYSCALL_DEFINE1(hello, int, n)
{
    int i;
    long sum = 0;

    for (i = 1; i <= n; i++) {
        sum += i;
        printk(KERN_INFO "i=%d sum=%ld\n", i, sum);   /* KERN_INFO 後面「沒有逗號」 */
    }
    return sum;
}
```

其他檔案:`include/linux/syscalls.h` → `asmlinkage long sys_hello(int n);`。其他檔案都不用動。

user 端 `test.c`(✅ 已通過 gcc 檢查)
```c
#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
int main(void) {
    int n = 5;
    long r = syscall(462, n);
    printf("sum(1..%d) = %ld\n", n, r);
    return 0;
}
```

**容易踩的坑**
- 寫成 `printk(KERN_INFO, "...")`:錯。`KERN_INFO` 是字串巨集,要直接和格式字串**相連**,中間沒有逗號。
- 印 `long` 時,kernel 的 printk 也要用 `%ld`。用錯會出現編譯警告(kernel 編譯可能把警告當錯誤)。
- 回傳型別:`SYSCALL_DEFINE` 產生的函式回傳 `long`,所以 `return sum;` 沒問題。
- 換成 n! 時,n 太大(大於 20)會溢位;考試的 n 都很小,不用擔心。

---

### 題 4(可能性:**中**)流程/填空/口試題:新增 syscall 要改哪些檔案

**題目(筆試或口頭 demo 時問)**:要新增一個名為 `hello`、編號 462 的 syscall,請寫出:
- (a) 要新增或修改哪些檔案,各加哪一行;
- (b) 從編譯到測試的指令順序;
- (c) `./test` 和 `sudo dmesg | tail` 各會看到什麼。

- **變形來源**:p.12~p.18(Create a New Syscall 1~8)、p.21~p.32(Compiling and installing)、p.34~p.39(Testing)。
- **可能性與理由**:上機時間有限,助教在 demo 時順便口頭追問是很常見的做法。Ex1(60 分)本身就是「完成範例步驟並 demo」(p.41)。

**參考答案**

(a) 要改的 5 個檔案

| # | 檔案(相對 `~/linux`) | 加入的行 | 作用 |
|---|---|---|---|
| 1 | `hello/hello.c`(新檔) | `SYSCALL_DEFINE0(hello){ pr_info("Hello\n"); return 0; }` | syscall 本體 |
| 2 | `hello/Makefile`(新檔) | `obj-y += hello.o` | 把 hello.o **靜態編進 kernel** |
| 3 | `include/linux/syscalls.h` | `asmlinkage long sys_hello(void);`(加在 `#ifndef CONFIG_ARCH_HAS_SYSCALL_WRAPPER` 下面) | 宣告原型;`asmlinkage` 表示參數從 stack 取 |
| 4 | `Kbuild`(頂層) | `obj-y += hello/` | 讓建置系統去編譯 hello 資料夾 |
| 5 | `arch/x86/entry/syscalls/syscall_64.tbl` | `462 common hello sys_hello` | 登記編號。欄位依序是:編號、ABI(common)、名稱、進入函式 |

(b) 指令順序:見文末「考前 5 分鐘速查」。

(c) `./test` 印出 `0`(syscall 的回傳值);`sudo dmesg | tail` 最後一行是 `[ 時間 ] Hello`。

**容易踩的坑**
- 把 `obj-y += hello/` 寫進 `hello/Makefile`,或把 `obj-y += hello.o` 寫進 Kbuild:**兩個檔案寫反了**。
  - 子目錄的 Makefile 寫 `.o`。
  - 頂層的 Kbuild 寫 `資料夾/`。
- 漏寫 Kbuild 那一行:hello.c 根本不會被編譯,連結時找不到 `sys_hello`(或 syscall 不存在)。
- `obj-y` 和 `obj-m` 搞混:`obj-m` 是 Lab5 的 kernel module。
- Makefile 的 command 前面要用 TAB(p.11)。

---

### 題 5(可能性:**中低**)新增第二個 syscall(編號 463):兩數相加

**題目**:保留原本的 hello(462),再新增一個 `add` syscall(編號 463),接收兩個整數,在 dmesg 印出 `add: 10 + 20 = 30` 並回傳和。

**預期輸出**
```
$ ./test
hello: 0
add  : 30
$ sudo dmesg | tail -2
[  xxx.xxxxxx] Hello
[  xxx.xxxxxx] add: 10 + 20 = 30
```

- **變形來源**:p.12~p.18 全部步驟「再走一次」。這符合共通出題邏輯 3「把所有小節串在一起」。
- **為什麼可能性中低**:要動 syscalls.h 和 syscall_64.tbl,會觸發大量重編,上機時間可能不夠;比較可能出現在筆試的流程題。不過如果考試時間很長,這是最能驗證「真的懂每個檔案作用」的題目。
- ⚠️ **編號提醒**:新版 kernel 的 463 可能也已經被占用(見第 0 節)。要先 `grep` tbl 確認,用沒被用過的號碼。

**參考答案**(⚠️ kernel 端無法在此環境驗證)

最省事的做法是寫在**同一個** `hello/hello.c` 裡。一個 .c 可以定義多個 syscall,而且 `hello/Makefile` 和 Kbuild 都不用改。
```c
#include <linux/kernel.h>
#include <linux/syscalls.h>

SYSCALL_DEFINE0(hello)
{
    pr_info("Hello\n");
    return 0;
}

SYSCALL_DEFINE2(add, int, a, int, b)
{
    pr_info("add: %d + %d = %d\n", a, b, a + b);
    return a + b;
}
```

| 檔案 | 要加的行 |
|---|---|
| `include/linux/syscalls.h` | `asmlinkage long sys_add(int a, int b);`(加在 sys_hello 下一行) |
| `arch/x86/entry/syscalls/syscall_64.tbl` | `463 common add sys_add`(加在 462 下一行) |
| `hello/Makefile` | 不變(`obj-y += hello.o`)。如果另外開 `add.c`,要改成 `obj-y += hello.o add.o` |
| `Kbuild` | 不變(`obj-y += hello/`) |

user 端 `test.c`(✅ 已通過 gcc 檢查)
```c
#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
int main(void) {
    printf("hello: %ld\n", syscall(462));
    printf("add  : %ld\n", syscall(463, 10, 20));
    return 0;
}
```

**容易踩的坑**
- tbl 第 4 欄要寫 `sys_add`(進入函式),第 3 欄才寫 `add`。`SYSCALL_DEFINE2(add, ...)` 會自動產生 `sys_add`,三個地方的名字必須一致。
- syscall 名字可能撞到 kernel 既有的 syscall(例如名字取成 `sum` 或 `add` 時,先 `grep -w` tbl 確認沒有重名)。重名會讓連結失敗。
- 新號碼不能放進 512~547 的 x32 專用區(tbl 註解寫的)。

---

### 題 6(可能性:**低**)參數檢查:不合法時回傳錯誤

**題目**:沿用 Ex2(印 n 次 hello)。當 n ≤ 0 或 n > 10 時不印任何東西,回傳 `-EINVAL`。user 端分別呼叫 `syscall(462, 3)` 和 `syscall(462, -1)`,失敗時用 `perror` 印出原因。

**預期輸出**
```
$ ./test
n=3  -> 0
syscall: Invalid argument
n=-1 -> -1
$ sudo dmesg | tail -3
[  xxx.xxxxxx] hello
[  xxx.xxxxxx] hello
[  xxx.xxxxxx] hello
```

`perror` 走 stderr,而且沒有緩衝,所以 terminal 上可能先出現 `syscall: Invalid argument` 再出現 `n=-1 -> -1`;重新導向時順序也可能不同。

- **變形來源**:p.38 備註(「成功回傳 0 或正數,失敗回傳 -1」),加上 p.42 Ex2。
- **為什麼可能性低**:`-EINVAL` / errno 在 Lab 2 簡報裡**沒有明講**,只出現在 Lab 5(module init 回傳 `-EINVAL`)。依「不考簡報沒教的」原則,把它列為低。但它可以驗證 p.38「失敗回傳 -1」這句話,所以留作備案。

**參考答案**(⚠️ kernel 端無法在此環境驗證)
```c
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/errno.h>        /* EINVAL;其實 syscalls.h 已間接引入,寫上較保險 */

SYSCALL_DEFINE1(hello, int, n)
{
    int i;

    if (n <= 0 || n > 10)
        return -EINVAL;         /* kernel 回傳「負的錯誤碼」 */
    for (i = 0; i < n; i++)
        pr_info("hello\n");
    return 0;
}
```

其他檔案:同題 1(syscalls.h 改成 `asmlinkage long sys_hello(int n);`)。

user 端 `test.c`(✅ 已通過 gcc 檢查)
```c
#include <stdio.h>
#include <errno.h>
#include <sys/syscall.h>
#include <unistd.h>
int main(void) {
    long r;
    r = syscall(462, 3);
    printf("n=3  -> %ld\n", r);
    r = syscall(462, -1);
    printf("n=-1 -> %ld\n", r);
    if (r == -1)
        perror("syscall");
    return 0;
}
```

**容易踩的坑**
- kernel 要回傳 `-EINVAL`(負號),不是 `EINVAL`。少了負號,user 端會收到正數 22,看起來像成功。
- user 端看到的是 `-1`,錯誤碼在 `errno`(22),不是直接收到 -22。
- `perror` 前面不能有別的函式呼叫改動 errno。`printf` 成功時不會改 errno,但最好先存起來。

---

## 3. 共通的坑(所有題目都適用)

1. **一定要 reboot 並選新 kernel**:開機時按住 Shift → `Advanced options for Ubuntu` → `Linux 7.1.3-dirty`(p.34~p.36)。之後用 `uname -r` 確認。
2. **舊 kernel 下的症狀不一定是 ENOSYS**:如果 462 在原生 kernel 裡已經是別的 syscall(例如 mseal),跑在舊 kernel 時會呼叫到那個 syscall,可能回傳 -1(EINVAL)甚至 0。在這個環境(6.18)實測:`syscall(462, 3)` 回傳 -1、errno = EINVAL。**不要只靠回傳值判斷,要看 dmesg 有沒有出現你印的字**。
3. kernel 程式裡**不能**用 `<stdio.h>`、`printf`、`malloc`;只能用 `<linux/...>` 標頭和 `pr_info` / `printk`。
4. `SYSCALL_DEFINEn(name, 型別1, 名稱1, 型別2, 名稱2, ...)`:n = 參數組數;name 不要加 `sys_` 前綴(巨集會自動加上)。
5. 函式結尾要 `return`(回傳 long)。漏掉的話 user 端會拿到垃圾值。
6. 改完 hello.c 一定要依序跑 `make -j $(nproc)` → `sudo make -j $(nproc) modules_install` → `sudo make install` → `reboot`(p.43)。
7. 編譯錯誤訊息被洗掉時,改用 `make -j1` 看錯誤(p.24、p.22 備註)。
8. 所有 make 指令都要在 `~/linux` 底下執行(p.9)。

---

## 4. 考前 5 分鐘速查

### 4.1 要改的檔案(路徑相對 `~/linux`)
```
hello/hello.c                              SYSCALL_DEFINEn(hello, 型別, 名稱, ...) { pr_info("...\n"); return ...; }
hello/Makefile                             obj-y += hello.o
include/linux/syscalls.h                   asmlinkage long sys_hello(參數列);     ← #ifndef CONFIG_ARCH_HAS_SYSCALL_WRAPPER 下方,記得分號
Kbuild                                     obj-y += hello/                       ← 檔尾
arch/x86/entry/syscalls/syscall_64.tbl     462 common hello sys_hello            ← 先 grep 確認 462 沒被占用
```
只改 syscall 的行為(題 1~3、6)→ 只需改 `hello.c`,`syscalls.h` 建議同步改。
新增 syscall(題 5)→ 改 `hello.c` + `syscalls.h` + `syscall_64.tbl`。

### 4.2 hello.c 骨架
```c
#include <linux/kernel.h>
#include <linux/syscalls.h>

SYSCALL_DEFINE1(hello, int, n)        /* 0 參數:SYSCALL_DEFINE0(hello)   2 參數:SYSCALL_DEFINE2(hello, int, a, int, b) */
{
    int i;
    for (i = 0; i < n; i++)
        pr_info("hello\n");           /* 或 printk(KERN_INFO "hello\n"); */
    return 0;
}
```

### 4.3 test.c 骨架
```c
#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
int main(void) {
    printf("%ld\n", syscall(462, 3));
    return 0;
}
```

### 4.4 指令順序
```bash
# (第一次才需要)
sudo apt update
sudo apt-get install build-essential vim git cscope libncurses-dev libssl-dev bison flex
git clone --depth=1 -b v7.1.3 https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git
cd linux && make mrproper && mkdir hello && cd hello && touch hello.c Makefile
#  → 改 5 個檔案
cd ~/linux
cp /boot/config-$(uname -r) .config
make olddefconfig
make localmodconfig            # 一路 Enter

# (每次修改後都要做)
cd ~/linux
make -j $(nproc)               # 缺 gelf.h/dwarf.h → sudo apt install libelf-dev libdw-dev;gawk not found → sudo apt install gawk
                               # 憑證錯誤 → make menuconfig 清空 X.509 路徑、取消 revocation;grep SYSTEM_TRUSTED_KEYS .config 應為 ""
sudo make -j $(nproc) modules_install
sudo make install
reboot                         # 按住 Shift → Advanced options for Ubuntu → Linux 7.1.3-dirty

# 測試
uname -r                       # 7.1.3-dirty
make test                      # 或 gcc test.c -o test
chmod +x test
./test                         # terminal 只看到回傳值
sudo dmesg | tail              # kernel 的輸出在這裡
```

### 4.5 一句話觀念
- **syscall**:user space 向 kernel 請求服務的機制。libc 的 `write()` 只是包裝,真正執行的是 `sys_write()`(p.4、p.5)。
- **`obj-y`**:靜態編進 kernel。**Kbuild** 是 kernel 的建置系統。**`asmlinkage`**:參數從 stack 取(依講者備註)。
- **`common`**:所有共同支援的 ABI。tbl 欄位依序是:編號、ABI、名稱、進入函式。
- **`syscall()`**:成功回傳 ≥ 0,失敗回傳 -1(p.38 備註)。**`pr_info`** = `printk(KERN_INFO ...)`,輸出到 kernel log,用 dmesg 看。
