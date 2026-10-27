# Lab2_g1

> 來源:review/predict/Lab2_predict.md

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
