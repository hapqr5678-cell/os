# Lab6_g4

> 來源:review/predict/Lab6_predict.md

### 第 4 題：用迴圈 fork N 個 child（扇形樹），parent wait N 次並印出回收的 child PID【可能性：中】

**題目**：parent 用 for 迴圈產生 3 個 child。每個 child 印出「I am child process i.」以及 PID、PPID 後結束。parent 呼叫 3 次 `wait()`，每次印出結束的是哪一個 child（wait 的回傳值），最後印出自己的資料。

```
          P
        / | \
      C1  C2  C3
```
輸出範例：
```
I am child process 2.	Pid: 2135	Parent Pid: 2133
I am child process 1.	Pid: 2134	Parent Pid: 2133
I am child process 3.	Pid: 2136	Parent Pid: 2133
Parent: child 2134 terminated
Parent: child 2135 terminated
Parent: child 2136 terminated
I'm parent process.	Pid: 2133	Parent Pid: 2090
```

**變形自**：p15 的「fork() multiple times」範例（兩個 `if(fork()==0)` 加上兩次 `wait(NULL)`），推廣成迴圈。另外用到 p14「wait 回傳該 child 的 PID」。
**可能性中的理由**：p15 的範例和截圖（I am child process 1/2、I'm parent process）就是這個樣子，只是改用迴圈、改變 N。另外它也是「多分支樹」的通用寫法。

**參考答案**
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define N 3

int main(void)
{
    pid_t pid;
    int i;

    for (i = 1; i <= N; i++) {
        pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        }
        if (pid == 0) {                              /* child i */
            printf("I am child process %d.\tPid: %d\tParent Pid: %d\n",
                   i, getpid(), getppid());
            exit(0);                                 /* 一定要 exit，否則 child 會繼續跑迴圈 */
        }
    }

    for (i = 0; i < N; i++) {                        /* 有 N 個 child 就 wait N 次 */
        pid = wait(NULL);
        printf("Parent: child %d terminated\n", pid);
    }
    printf("I'm parent process.\tPid: %d\tParent Pid: %d\n", getpid(), getppid());
    return 0;
}
```

✅ 已驗證：輸出如上。3 個 child 的 Parent Pid 都是 2133，等於 parent 的 PID。child 的列印順序不固定（這次是 2→1→3），parent 一定最後印。

**容易踩的坑**
- **child 沒有 `exit(0)`**：child 會繼續跑迴圈，自己也去 fork，總共變成 2³ = 8 個 process，其中 7 個是 child。✅ 已驗證：把 exit 拿掉並關掉 buffer 後，"I am child" 印了 7 次，"I'm parent" 印了 8 次，而且出現 17 次 `child -1 terminated`，也就是對沒有 child 的 process 呼叫 wait 得到 -1。
- 把 `wait` 寫在 fork 迴圈裡面（fork 一個、wait 一個）：結果還是對的，但 child 變成依序執行，不是同時執行。題目如果沒有要求，兩種寫法都可以，但「先全部 fork 完再 wait」比較符合 p15 的寫法。
- 迴圈變數 `i` 在 child 裡是 fork 當下的值，所以 child 知道自己是第幾個。這也是「memory image 被複製」的應用。
