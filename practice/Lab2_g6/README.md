# Lab2_g6

> 來源:review/predict/Lab2_predict.md

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
