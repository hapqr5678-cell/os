# Lab6_g3

> 來源:review/predict/Lab6_predict.md

### 第 3 題：fork–exec，child 用 execlp 執行 `ls -l`，parent 等待後印訊息【可能性：中高】

**題目**：parent fork 出一個 child。child 印出自己的 PID、PPID，然後用 `execlp` 執行 `ls -l`。parent 用 `wait()` 等 child 結束後，印出自己的 PID、child 的 PID，以及 `wait()` 的回傳值。

輸出範例：
```
Parent start: PID = 2116
Child: PID = 2118, PPID = 2116, now exec "ls -l"
total 0
-rw-r--r-- 1 root root 0 Oct 27 12:02 a.txt
-rw-r--r-- 1 root root 0 Oct 27 12:02 b.c

------ this is parent process ------
Pid : 2116	child pid : 2118	wait() returned : 2118
------------------------------------
```

**變形自**：p20 的 exec Example，加上 p16 的「parent waits for child and executes last」。考的觀念是 p17–p19：exec 會用新程式**覆蓋** child 的 image，PID 不變，exec 成功後就不會返回；以及 fork–exec 組合。
**可能性中高的理由**：exec 是這個 lab 唯一教了卻沒在 exercise 裡考的機制，最適合拿來做變形。p20 的原始範例沒有 wait，所以輸出順序不保證（截圖剛好是 ls 先出現）。加上 wait 就能和 Ex2 的觀念串在一起。也可能換成其他指令，例如 `execlp("ps", "ps", NULL)`、`execlp("echo", "echo", "hello", NULL)`、`execlp("date", "date", NULL)`，寫法都一樣。

**參考答案**
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void)
{
    pid_t pid, done;

    printf("Parent start: PID = %d\n", getpid());
    fflush(stdout);

    pid = fork();
    switch (pid) {
    case -1:
        perror("fork");
        exit(1);
    case 0:                                          /* child */
        printf("Child: PID = %d, PPID = %d, now exec \"ls -l\"\n",
               getpid(), getppid());
        fflush(stdout);                              /* exec 前也要清 buffer！ */
        execlp("ls", "ls", "-l", NULL);              /* 成功就不會回來 */
        perror("execlp");                            /* 只有失敗才會跑到這 */
        exit(1);
    default:                                         /* parent */
        done = wait(NULL);                           /* 回傳結束的 child PID */
        printf("\n------ this is parent process ------\n");
        printf("Pid : %d\tchild pid : %d\twait() returned : %d\n",
               getpid(), pid, done);
        printf("------------------------------------\n");
    }
    return 0;
}
```

✅ 已驗證：在含 `a.txt`、`b.c` 的目錄下執行，輸出如上。`wait()` 的回傳值（2118）等於 fork 回傳給 parent 的 child PID（2118）。parent 的訊息一定出現在 ls 輸出之後。

**容易踩的坑**
- **execlp 參數**：第 1 個是要找的程式（有 `p`，所以寫 `"ls"` 就會去 PATH 找；p20 寫 `"/bin/ls"` 也可以）。第 2 個是 argv[0]，通常和程式名稱相同。**最後一定要放 `NULL`**，少了會出現未定義行為。
- **exec 之後的程式碼不會執行**：如果在 execlp 後面寫 `printf("done")`，只有 exec 失敗時才會印出來。所以 execlp 後面要寫 `perror` 加上 `exit(1)`。如果沒有 exit，exec 失敗時 child 會繼續跑 parent 的程式碼。
- **exec 前沒有 fflush，child 的訊息會不見**：exec 會把整個 image 換掉，stdio buffer 裡還沒寫出的資料就消失了。✅ 已驗證：拿掉 child 的 fflush 後，用 `./a.out | cat` 執行，"Child: PID = …" 那一行完全沒有出現（在終端機執行則正常）。
- 如果 parent 沒有 wait（照抄 p20），parent 的訊息可能先印，甚至 shell 的提示字元會夾在 ls 輸出中間。
- p20 原始碼裡的 `chile pid` 是投影片的拼字錯誤，不要照抄。
