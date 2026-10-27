# Lab6_g6

> 來源:review/predict/Lab6_predict.md

### 第 6 題：fork() 連續呼叫 n 次，總共有幾個 process？【可能性：低～中】

**題目**：執行下面的程式，回答總共會印出幾行？畫出 process tree。（或者反過來：寫程式讓每個 process 都印出 PID/PPID，再數一數總共有幾個。）
```c
for (i = 0; i < 3; i++)
    fork();
printf("PID: %-8d , PPID: %d\n", getpid(), getppid());
```

**答案**：每呼叫一次 fork，process 數就乘以 2，所以總共 **2³ = 8** 個 process，印 8 行。樹的形狀（P0 是原始 process）：
```
P0 ─┬─ P1 ─┬─ P3 ── P7
    │      └─ P4
    ├─ P2 ── P5
    └─ P6
（P0 有 3 個 child，第一個 child 有 2 個，依此類推：3, 2, 1, 1, 0, 0, 0, 0）
```

**變形自**：p15（fork 多次）、p8（fork 後兩個 process 都從下一行繼續執行）、p6（process tree）。
**可能性低～中的理由**：這是課本的經典題。但 Lab 6 的考試是上機寫程式，比較可能把它包裝成「寫程式驗證你的答案」。

**參考答案**
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void)
{
    int i;

    for (i = 0; i < 3; i++)
        fork();                         /* 每呼叫一次，process 數 ×2 */

    printf("PID: %-8d , PPID: %d\n", getpid(), getppid());
    fflush(stdout);
    while (wait(NULL) > 0)              /* 不知道有幾個 child：等到 wait 回 -1 */
        ;
    return 0;
}
```

✅ 已驗證：剛好印 8 行，沒有重複的 PID。從 PPID 還原出的樹：2194 有 3 個 child（2197、2198、2199），2197 有 2 個（2200、2201），2198 有 1 個（2203），2200 有 1 個（2202），跟上面畫的形狀一致。
```
PID: 2194     , PPID: 2090
PID: 2199     , PPID: 2194
PID: 2197     , PPID: 2194
PID: 2200     , PPID: 2197
PID: 2202     , PPID: 2200
PID: 2198     , PPID: 2194
PID: 2203     , PPID: 2198
PID: 2201     , PPID: 2197
```

**容易踩的坑**
- 回答 4 或 6 都是錯的。n 次 fork 會有 **2ⁿ 個 process**，其中 **2ⁿ − 1 個是新建的 child**。
- 如果在 fork 迴圈前面有 printf 但沒有 fflush，又導向到檔案，那一行會被複製成 8 份。
- 如果沒有 wait，有些 child 印出來時 parent 已經結束，PPID 就會變成 1 或 systemd，樹會還原錯誤。用 `while (wait(NULL) > 0);` 可以等完「不知道有幾個」的 child。這是 p14「沒有 child 時 wait 回傳 -1」的應用。
