# Lab2_g3

> 來源:review/predict/Lab2_predict.md

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
