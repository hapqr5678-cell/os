# Lab6_g5

> 來源:review/predict/Lab6_predict.md

### 第 5 題：鏈狀 process tree A → B → C → D【可能性：中】

**題目**：建立 4 個 process，形成一條鏈：A 產生 B，B 產生 C，C 產生 D。每個 process 都要先 wait 自己的 child，再印出自己的名稱、PID、PPID。

```
A
└── B
    └── C
        └── D
```
輸出範例（一定是由下往上印）：
```
Process D -	PID: 1915     , PPID: 1914
Process C -	PID: 1914     , PPID: 1913
Process B -	PID: 1913     , PPID: 1912
Process A -	PID: 1912     , PPID: 1887
```

**變形自**：Exercise 2（p25–p27）的另一種樹形，加上 p6 的 process tree 概念（child 再產生 child）。
**可能性中的理由**：鏈狀樹和扇形樹（第 4 題）是兩種最極端的形狀，出題成本低。它考的重點跟扇形相反：是 **child** 去 fork，不是 parent 一直 fork。

**參考答案**
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/*  A -> B -> C -> D  （一條鏈，每個 process 只有 1 個 child） */
int main(void)
{
    char name = 'A';
    int depth = 0;
    pid_t pid;

    while (depth < 3) {                 /* A、B、C 各 fork 一次 */
        pid = fork();
        if (pid < 0) { perror("fork"); exit(1); }
        if (pid > 0) {                  /* 我是 parent：等 child 後跳出 */
            wait(NULL);
            break;
        }
        name++;                         /* 我是 child：變成下一層，繼續 fork */
        depth++;
    }
    printf("Process %c -\tPID: %-8d , PPID: %d\n", name, getpid(), getppid());
    return 0;
}
```
（不用迴圈也可以：照 ex2.c 的風格巢狀寫 3 層 `if (fork()==0) { … }`。考試時用哪種都行，巢狀寫法比較直觀。）

✅ 已驗證（PPID 檢查 OK：D 的 PPID 等於 C 的 PID，C 的等於 B 的，B 的等於 A 的。執行多次順序都固定是 D→C→B→A）：
```
Process D -	PID: 1817     , PPID: 1816
Process C -	PID: 1816     , PPID: 1815
Process B -	PID: 1815     , PPID: 1812
Process A -	PID: 1812     , PPID: 1887
```

**容易踩的坑**
- 迴圈裡判斷反了：如果在 `pid == 0` 時 break，會變成 A 自己一直 fork，結果是扇形樹而不是鏈。
- parent 拿到 pid 之後沒有 `break`：parent 會繼續迴圈，再 fork 一個 child，樹就多出分支。
- 因為每一層都會 wait，所以輸出順序是固定的（從最深的開始）。如果順序不是 D→C→B→A，就表示 wait 寫錯了。
