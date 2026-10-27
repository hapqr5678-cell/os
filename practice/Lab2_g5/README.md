# Lab2_g5

> 來源:review/predict/Lab2_predict.md

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
