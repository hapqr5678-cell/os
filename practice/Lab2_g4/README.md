# Lab2_g4

> 來源:review/predict/Lab2_predict.md

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
