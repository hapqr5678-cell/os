# Lab6_g1

> 來源:review/predict/Lab6_predict.md

### 第 1 題：換一棵 process tree（6 個 process）【可能性：高】

**題目**：寫一個程式建立 6 個 process，形成下面這棵樹。每個 process 印出自己的 PID 和 parent 的 PID。有 child 的 process 必須依照 child 的數目呼叫相同次數的 `wait()`，等所有 child 結束後才印自己的資料。

```
            A
          /   \
         B     C
        / \     \
       D   E     F
```
輸出範例（PID 會不同，順序可能不同，但 parent 一定在自己所有 child 之後）：
```
Child process D -   PID: 1891     , PPID: 1889
Child process E -   PID: 1892     , PPID: 1889
Child process F -   PID: 1893     , PPID: 1890
Child process B -   PID: 1889     , PPID: 1888
Child process C -   PID: 1890     , PPID: 1888
Parent process A -  PID: 1888     , PPID: 1887
```

**變形自**：Exercise 2（p25–p27）。用到 fork（p8）、getpid/getppid（p9）、wait（p14），以及 p15/p16 的「wait 次數 = child 數、parent 最後執行」。
**可能性高的理由**：Ex2 佔 50 分，是整個 lab 的重心。換一棵樹不需要新的 API，助教最容易出，也最容易用 PPID 改考卷。也可能出鏈狀樹（第 5 題）或扇形樹（第 4 題）。

**參考答案**
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void report(char name, const char *role)
{
    printf("%s %c -\tPID: %-8d , PPID: %d\n", role, name, getpid(), getppid());
    fflush(stdout);
}

int main(void)
{
    pid_t pid;
    int i;

    /* A 產生 B */
    pid = fork();
    if (pid < 0) { perror("fork"); exit(1); }
    if (pid == 0) {                          /* B */
        pid = fork();                        /* B 產生 D */
        if (pid < 0) { perror("fork"); exit(1); }
        if (pid == 0) { report('D', "Child process"); exit(0); }
        pid = fork();                        /* B 產生 E */
        if (pid < 0) { perror("fork"); exit(1); }
        if (pid == 0) { report('E', "Child process"); exit(0); }
        for (i = 0; i < 2; i++) wait(NULL);  /* B 有 2 個 child */
        report('B', "Child process");
        exit(0);
    }

    /* A 產生 C */
    pid = fork();
    if (pid < 0) { perror("fork"); exit(1); }
    if (pid == 0) {                          /* C */
        pid = fork();                        /* C 產生 F */
        if (pid < 0) { perror("fork"); exit(1); }
        if (pid == 0) { report('F', "Child process"); exit(0); }
        wait(NULL);                          /* C 有 1 個 child */
        report('C', "Child process");
        exit(0);
    }

    for (i = 0; i < 2; i++) wait(NULL);      /* A 有 2 個 child */
    report('A', "Parent process");
    return 0;
}
```

**解題心法（任何樹都適用）**
1. 先寫出每個節點有幾個 child，這個數字就是它的 wait 次數：A=2、B=2、C=1、D/E/F=0。
2. 每個 `fork()` 後面緊接著寫 `if (pid == 0) { … exit(0); }`，把 child 要做的事整個關在裡面。
3. 葉節點：印完就 `exit(0)`。中間節點：在 child 區塊裡繼續 fork 它自己的 child，wait 完再印，然後 `exit(0)`。

✅ 已驗證（執行 3 次，PPID 檢查全部 OK；D、E 的順序會互換，但 B 一定在 D、E 之後，A 一定最後）：
```
Child process E -	PID: 1909     , PPID: 1905
Child process D -	PID: 1907     , PPID: 1905
Child process F -	PID: 1908     , PPID: 1906
Child process B -	PID: 1905     , PPID: 1904
Child process C -	PID: 1906     , PPID: 1904
Parent process A -	PID: 1904     , PPID: 1887
PPID check: OK  (D,E 的 PPID = B 的 PID；F 的 PPID = C；B,C 的 PPID = A)
```

**容易踩的坑**
- **child 區塊忘了 `exit(0)`**：child 會繼續往下跑 parent 的程式碼，又 fork 出更多 process，樹的形狀就錯了。這是最常見的錯誤。
- **wait 次數少了**：parent 可能比 child 先印，甚至先結束。這時 child 會變成孤兒，印出來的 PPID 會變成 1 或 systemd 的 PID，樹看起來就錯了。
- **wait 次數多了**：多出來的 `wait()` 只會回傳 -1，不會卡住，但不符合「依 child 數呼叫」的要求，可能被扣分。
- **把 fork 寫在 if 條件裡又在別處用到 pid**（例如 `if (fork()==0)`）：parent 拿不到 child PID。這題用不到，但要印 child PID 的題目就會出問題。
