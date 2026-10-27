# Lab 2 複習：Adding a New System Call

> 來源：Lab2.pptx（45 張投影片，含講者備註）。practice/ 沒有 lab2 資料夾。

---

## 1. 這個 LAB 在學什麼

下載 Linux kernel 原始碼，**自己新增一個 system call**（`sys_hello`，編號 462），重新編譯、安裝 kernel、從 GRUB 開進新 kernel，再寫 user program 用 `syscall(462)` 呼叫它，最後用 `dmesg` 看 kernel 印出的訊息。Exercise 2 要把它改成可以吃一個參數（印 n 次 hello）。

---

## 2. 核心觀念

### 2.1 什麼是 System Call
- **System call 是 user-space 程式向 kernel 請求服務的機制**。user program 不能直接碰硬體或 kernel 資料，必須透過 system call 進入 kernel mode。
- 例子：Process 管理 `fork() exec() exit() wait()`；檔案操作 `open() read() write() close()`。
- 呼叫路徑（投影片圖）：
  ```
  User Program  ──write()──►  C library (libc write())  ──System Call──►  Kernel (sys_write())
           User Space                          ｜                  Kernel Space
  ```
  libc 的 `write()` 只是包裝（wrapper），真正做事的是 kernel 裡的 `sys_write()`。

### 2.2 新增 syscall 需要動的 4 個地方（必背）
| 步驟 | 檔案 | 加入的內容 | 為什麼 |
|---|---|---|---|
| ① 實作 | `linux/hello/hello.c` | `SYSCALL_DEFINE0(hello){...}` | syscall 本體 |
| ② 子目錄 Makefile | `linux/hello/Makefile` | `obj-y += hello.o` | `obj-y` = **直接編進 kernel 映像檔**（不是 module） |
| ③ 宣告 | `linux/include/linux/syscalls.h` | `asmlinkage long sys_hello(void);` | 集中宣告所有 syscall 原型 |
| ④ 頂層建置 | `linux/Kbuild` | `obj-y += hello/` | 讓 Kbuild 也去編譯 hello 資料夾 |
| ⑤ syscall table | `arch/x86/entry/syscalls/syscall_64.tbl` | `462 common hello sys_hello` | 註冊編號，user space 呼叫 462 時 kernel 才找得到對應函式 |

### 2.3 重要名詞
- **`asmlinkage`**：告訴編譯器這個函式的參數要從 **stack** 取得，而不是暫存器（投影片備註的說法）。syscall 進入點都會加。
- **`obj-y` vs `obj-m`**：`obj-y` 把東西**靜態編進 kernel**（開機就常駐）；`obj-m` 編成可載入的 **module (.ko)**（Lab5 會用）。
- **Kbuild**：Kernel Build System，以 Makefile 為基礎的 kernel 建置框架；每個子目錄有自己的 Kbuild/Makefile，由最上層 Makefile 統籌。
- **syscall table 欄位**：`462`=syscall 編號、`common`=x86 的 64-bit 與 x32 ABI 都適用（投影片備註說「所有共同支援的架構」）、`hello`=名字、`sys_hello`=進入函式名稱。
- **`SYSCALL_DEFINEn(name, type1, arg1, ...)`**：定義有 n 個參數的 syscall 的巨集；會自動產生 `sys_name` 函式。`SYSCALL_DEFINE0(hello)` → `sys_hello(void)`。
- **`pr_info()`**：等於 `printk(KERN_INFO ...)`，把訊息寫進 **kernel log**（不是 terminal！），要用 `dmesg` 看。
- **GRUB**：GRand Unified Bootloader，開機載入程式，用來選擇要開哪一個 kernel。
- **`syscall()` 回傳值**：成功回傳 0 或正數，失敗回傳 -1。

### 2.4 Makefile 基本格式
```make
target: dependencies
<TAB>command
```
- target：要產生的東西（或 `clean` 這種虛擬目標）；dependencies：依賴的檔案；command：要執行的指令。
- **command 前面一定要是 TAB，不能是空白**（必考）。

### 2.5 編譯 kernel 的設定指令（看備註）
| 指令 | 作用 |
|---|---|
| `cp /boot/config-$(uname -r) .config` | 把目前正在跑的 kernel 設定檔複製過來當 `.config` |
| `make olddefconfig` | 保留舊設定，新版 kernel 新增的選項自動套預設值 |
| `make localmodconfig` | 依目前已載入的 modules 精簡 `.config`（編譯更快）；遇到新選項會詢問，`y`=編進 kernel、`m`=編成 module、`n`=不編，大寫是預設，直接 Enter |
| `make -j $(nproc)` | 用 CPU 核心數個 job 平行編譯 kernel（10+ 分鐘） |
| `sudo make -j $(nproc) modules_install` | 把 `.ko` 模組裝到 `/lib/modules/<版本>/` |
| `sudo make install` | 把 kernel 裝到 `/boot` 並更新 GRUB 設定 |
| `make mrproper` | 清除所有編譯產物與設定（徹底清乾淨） |

---

## 3. 指令／API 速查表

| 名稱 | 用途 | 重要參數／回傳值 | 範例 |
|---|---|---|---|
| `git clone --depth=1 -b v7.1.3 <url>` | 下載 kernel 原始碼 | `--depth=1` 只抓最新一版 | 見步驟 |
| `make mrproper` | 清理 | — | `make mrproper` |
| `make menuconfig` | 文字介面設定 kernel 選項 | 用來處理憑證錯誤 | — |
| `grep SYSTEM_TRUSTED_KEYS .config` | 確認憑證路徑已清空 | 應看到 `""` | — |
| `make -j $(nproc)` | 平行編譯 | `-j` = jobs；`$(nproc)` = 核心數 | — |
| `make -j1` | 單核編譯 | 錯誤訊息不會被洗掉，方便除錯 | — |
| `SYSCALL_DEFINE0(name)` | 定義無參數 syscall | — | `SYSCALL_DEFINE0(hello)` |
| `SYSCALL_DEFINE1(name, type, arg)` | 定義 1 個參數的 syscall | 型別和名稱用逗號分開 | `SYSCALL_DEFINE1(hello, int, n)` |
| `pr_info(fmt, ...)` / `printk(KERN_INFO ...)` | 印到 kernel log | — | `pr_info("Hello\n");` |
| `long syscall(long number, ...)` | 以編號直接呼叫 syscall | `#include <sys/syscall.h>`、`<unistd.h>`；成功 ≥0，失敗 -1 | `syscall(462)` |
| `dmesg` | 顯示 kernel 訊息 | 搭配 `tail` 看最後 10 行 | `sudo dmesg \| tail` |
| `uname -r` | 顯示目前 kernel 版本 | 開新 kernel 後應是 `7.1.3-dirty` | — |

---

## 4. 程式範例逐行解說

### 4.1 `linux/hello/hello.c`（投影片範例）
```c
#include <linux/kernel.h>
#include <linux/syscalls.h>

SYSCALL_DEFINE0(hello){
    pr_info("Hello\n");
    return 0;
}
```
- 引入的是 `<linux/...>` kernel 標頭，**不能用 `<stdio.h>`、`printf`**（kernel 裡沒有 C 標準函式庫）。
- `SYSCALL_DEFINE0(hello)` 展開後就是 `asmlinkage long sys_hello(void)`。
- `pr_info` 輸出到 kernel log；`return 0` 會變成 user 端 `syscall()` 的回傳值。

### 4.2 `linux/hello/Makefile`
```make
obj-y += hello.o
```
Kbuild 看到 `hello.o` 會自動在同資料夾找 `hello.c` 編譯並連進 kernel。

### 4.3 `include/linux/syscalls.h`（加在 `#ifndef CONFIG_ARCH_HAS_SYSCALL_WRAPPER` 區段）
```c
asmlinkage long sys_hello(void);
```

### 4.4 `linux/Kbuild`（檔尾加一行）
```make
obj-y += hello/
```

### 4.5 `arch/x86/entry/syscalls/syscall_64.tbl`
```
462   common   hello   sys_hello
```
加在 461（`lsm_list_modules`）後面。注意 table 註解說 512~547 是 x32 專用區，不要把新 syscall 放那裡。

### 4.6 測試程式 test.c
```c
#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
int main(void) {
    printf("%ld\n", syscall(462));
    return 0;
}
```
- `syscall()` 回傳 `long`，所以用 `%ld`。
- 執行 `./test` → terminal 印出 `0`（syscall 的回傳值）。
- **`Hello` 不會出現在 terminal**，要 `sudo dmesg | tail` 才會看到 `[ 684.324225] Hello`。
- 若在**舊 kernel**（沒有 462 的那個）執行，會印 `-1`（errno = ENOSYS，補充）。

編譯：投影片用 `make test`——沒有 Makefile 時，make 的內建規則會自動執行 `cc test.c -o test`。也可直接 `gcc test.c -o test`。

### 4.7 Exercise 2 參考解法：`SYSCALL_DEFINE1`
`hello/hello.c`：
```c
#include <linux/kernel.h>
#include <linux/syscalls.h>

SYSCALL_DEFINE1(hello, int, n){
    int i;
    for (i = 0; i < n; i++)
        pr_info("Hello\n");
    return 0;
}
```
`include/linux/syscalls.h` 的宣告要一起改：
```c
asmlinkage long sys_hello(int n);
```
syscall table 不用改（編號、名稱不變）。測試程式：
```c
#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
int main(void) {
    printf("%ld\n", syscall(462, 3));   /* 印 3 次 hello */
    return 0;
}
```
`sudo dmesg | tail` 應看到 3 行 `Hello`。
注意 `SYSCALL_DEFINE1(hello, int, n)` 的寫法是「**型別, 名稱**」分開成兩個參數，不是 `int n`。

---

## 5. 上機考實作步驟

```bash
# ===== 0. 安裝工具 =====
sudo apt update
sudo apt-get install build-essential vim git cscope libncurses-dev libssl-dev bison flex

# ===== 1. 下載 kernel 原始碼 =====
git clone --depth=1 -b v7.1.3 https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git
# (備註：Ubuntu 24.04 也可改抓 v6.8：git clone --depth=1 --branch v6.8 https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git)
cd linux

# ===== 2. 建立 syscall =====
make mrproper
mkdir hello && cd hello
vim hello.c            # 寫入 SYSCALL_DEFINE0(hello){ pr_info("Hello\n"); return 0; }
echo 'obj-y += hello.o' > Makefile
cd ..
vim include/linux/syscalls.h       # 加 asmlinkage long sys_hello(void);
vim Kbuild                          # 檔尾加 obj-y += hello/
vim arch/x86/entry/syscalls/syscall_64.tbl   # 加 462 common hello sys_hello

# ===== 3. 設定與編譯 =====
cp /boot/config-$(uname -r) .config
make olddefconfig
make localmodconfig     # 一路按 Enter 用預設值
make -j $(nproc)
#  - 缺 gelf.h / dwarf.h：sudo apt install libelf-dev libdw-dev
#  - gawk not found：     sudo apt install gawk
#  - 憑證錯誤：make menuconfig → Cryptographic API → Certificates for signature checking
#      → 清空 additional X.509 keys 路徑；取消 revocation certificates 的 *
#      → grep SYSTEM_TRUSTED_KEYS .config 確認為 ""
#  - 錯誤訊息太多看不到：make -j1

# ===== 4. 安裝 =====
sudo make -j $(nproc) modules_install
sudo make install
reboot
# 開機時按住 Shift 進 GRUB → Advanced options for Ubuntu → Linux 7.1.3-dirty

# ===== 5. 測試 =====
uname -r                 # 確認是 7.1.3-dirty
mkdir ~/syscall_test && cd ~/syscall_test
vim test.c
make test                # 或 gcc test.c -o test
chmod +x test
./test                   # 印出 0
sudo dmesg | tail        # 看到 Hello

# ===== Exercise 2：改完 hello.c 與 syscalls.h 後 =====
cd ~/linux
make -j $(nproc)
sudo make -j $(nproc) modules_install
sudo make install
reboot
```

---

## 6. 常見錯誤與陷阱

1. **以為 `pr_info` 會印在 terminal**：它寫到 kernel ring buffer，要用 `dmesg` 看。terminal 只看到 `syscall()` 的回傳值。
2. **沒開新 kernel**：reboot 後沒在 GRUB 選 `7.1.3-dirty`，`syscall(462)` 會回傳 `-1`。先 `uname -r` 確認。
3. **改了程式沒重編／重裝／重開機**：syscall 是編進 kernel 映像的（`obj-y`），每次修改都要 `make` → `modules_install` → `install` → `reboot` 才生效。
4. **Makefile 指令前用空白**：一定要 TAB。
5. **syscalls.h 宣告和 SYSCALL_DEFINE 參數不一致**（Exercise 2 改成一個參數卻沒改宣告）。
6. **`SYSCALL_DEFINE1(hello, int n)`** 少了逗號——巨集要求型別和名稱分開。
7. **在 kernel 程式用 `printf` / `#include <stdio.h>`**：kernel 沒有 libc。
8. **printf 用 `%d` 印 `syscall()` 回傳的 long**：會有警告，應用 `%ld`。
9. **編號衝突**：462 若已被新版 kernel 使用，就要換一個沒用過的編號（補充）。
10. 編 kernel 很慢 → 把 VM 的 CPU 核心數調高（接近實體邏輯處理器數）。
11. 所有 make 指令都要在 `linux/` 資料夾內執行。

---

## 7. 筆試模擬題

**Q1（選擇）** 下列何者最正確地描述 system call？
(A) C 標準函式庫中的函式　(B) user-space 程式向 kernel 請求服務的機制　(C) 編譯器提供的內建函式　(D) shell 指令
> **答：B**。libc 函式（如 `printf`）可能在內部呼叫 system call，但本身不是。

**Q2（選擇）** 在 kernel Makefile 中 `obj-y += hello.o` 的意思是？
(A) 編成可載入模組　(B) 不編譯　(C) 直接靜態編進 kernel 映像檔　(D) 只編成 .o 不連結
> **答：C**。`obj-m` 才是編成 module。

**Q3（簡答）** 新增一個 system call 需要修改哪些檔案？
> **答：** ① 新增實作檔 `hello/hello.c` 與 `hello/Makefile`（`obj-y += hello.o`）；② `include/linux/syscalls.h` 加 `asmlinkage long sys_hello(void);`；③ 頂層 `Kbuild` 加 `obj-y += hello/`；④ `arch/x86/entry/syscalls/syscall_64.tbl` 加 `462 common hello sys_hello`。

**Q4（輸出預測）** 在已安裝新 kernel 的系統執行 test.c（`printf("%ld\n", syscall(462));`），terminal 顯示什麼？`Hello` 在哪裡？
> **答：** terminal 印 `0`；`Hello` 在 kernel log，需 `sudo dmesg | tail` 查看。

**Q5（是非）** `asmlinkage` 表示函式參數從暫存器傳遞。
> **答：錯**。依投影片，`asmlinkage` 告訴編譯器參數從 **stack** 取得。

**Q6（選擇）** syscall table 那一行 `462 common hello sys_hello` 中 `sys_hello` 代表？
(A) syscall 編號　(B) ABI 類型　(C) syscall 名稱　(D) kernel 中的進入函式名稱
> **答：D**。

**Q7（簡答）** `make -j $(nproc)` 中 `-j` 與 `$(nproc)` 的意義？
> **答：** `-j` 指定同時執行的編譯 job 數；`$(nproc)` 取得 CPU 核心數，所以是「用全部核心平行編譯」。

**Q8（簡答）** `sudo make modules_install` 與 `sudo make install` 各做什麼？
> **答：** 前者把編好的 kernel modules（.ko）裝到 `/lib/modules/<版本>/`；後者把 kernel 映像裝到 `/boot` 並更新 GRUB 開機選單。

**Q9（程式填空）** 讓 syscall 接受一個整數 n 並印 n 次 Hello：
```c
SYSCALL_DEFINE1(______) {
    int i;
    for (i = 0; i < n; i++) pr_info("Hello\n");
    return 0;
}
```
> **答：** `hello, int, n`。

**Q10（是非）** 修改 hello.c 後，只要重新 `make` 不用重開機，新的 syscall 就會生效。
> **答：錯**。必須 `modules_install`、`install` 並 reboot 進新 kernel。

**Q11（選擇）** `make localmodconfig` 詢問新選項時直接按 Enter 會？
(A) 選 y　(B) 選 m　(C) 選大寫的預設值（通常是 N）　(D) 中止
> **答：C**。

**Q12（簡答）** 為何 user program 透過 `write()` 輸出時，實際會經過 C library？
> **答：** libc 的 `write()` 是 wrapper，負責準備參數、觸發 system call 進入 kernel，由 kernel 的 `sys_write()` 真正執行，再把結果（或 -1 與 errno）傳回。
