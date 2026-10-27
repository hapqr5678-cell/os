# Lab 6 複習：Process Control

> 來源：Lab6.pptx（29 張投影片，含講者備註）＋ practice/Lab6_ex1、Lab6_ex2/（ex1.c、ex2.c）

---

## 1. 這個 LAB 在學什麼

透過程式理解 **process** 的概念，以及 UNIX 用哪些 system call 控制 process：`fork()` 建立 child、`getpid()/getppid()` 取得 PID、`wait()` 讓 parent 等 child 結束、`exec` 系列（`execlp()`）讓 process 換成執行另一支程式。Exercise 1 觀察 fork 後 global／local 變數各自獨立；Exercise 2 建出指定形狀的 process tree。

---

## 2. 核心觀念

### 2.1 Process 是什麼
- **Process = 正在執行中的程式（a program in execution）**。
- Process 的記憶體分成（必考）：
  | 區段 | 內容 | 範例 |
  |---|---|---|
  | **text** section | 程式碼 | 編譯後的指令 |
  | **data** section | **全域變數** | `int a = 10;`（函式外） |
  | **heap** section | **動態配置**的記憶體 | `malloc()` 的空間 |
  | **stack** section | **暫存資料**：區域變數、函式參數、回傳位址 | 函式內 `int b = 20;` |
- Process 不只是程式碼，還包含**目前的執行狀態**：**program counter** 與 CPU **registers** 的內容。
- 每個 process 有自己的 base address／位址空間，彼此**記憶體隔離**，不會互相干擾。
- （補充）**program vs process**：program 是存在硬碟上的被動檔案；process 是載入記憶體、正在執行的主動實體。同一個 program 可以同時有多個 process。

### 2.2 Process tree
Parent process 建立 child process，child 又可以建立自己的 child，形成一棵 **process tree**。

### 2.3 PID
- UNIX 用唯一的整數 **process ID** 辨識每個 process。
- 每個 process 也有 **parent process ID（PPID）**，初始值就是建立它的那個 process 的 PID。
- `getpid()` 回傳自己的 PID；`getppid()` 回傳 parent 的 PID。

### 2.4 fork()（最重要）
- 建立一個新的 child process，**把 parent 的 memory image 複製一份給 child**。
- fork 之後，parent 和 child **都從 fork() 的下一行繼續執行**，各自在自己的記憶體中跑。
- **回傳值（必考）**：
  | 情況 | 回傳值 |
  |---|---|
  | 在 **parent** 中 | **child 的 PID**（> 0） |
  | 在 **child** 中 | **0**（不是真的 PID，只是告訴它「你是 child」；child 要知道自己 PID 要呼叫 `getpid()`） |
  | 失敗 | **-1**（沒有 child 被建立） |
- 因為記憶體是**複製**的，child 改變數**不影響** parent（反之亦然）——不管是 global 還是 local 變數。
- **執行順序不確定**：parent 和 child 誰先跑是由 **CPU scheduler** 決定；多個 child 的輸出順序是非決定性的（nondeterministic），可能有 race condition。

### 2.5 wait()
- Parent 呼叫 `wait()` 會**停下來（block）直到某一個 child 結束**。
- 回傳**結束的那個 child 的 PID**；錯誤（例如根本沒有 child）回傳 **-1**。
- `wait(NULL)` 表示不在乎 child 的結束狀態；`wait(&status)` 可取得狀態。
- **一次 wait() 只處理一個 child**：有幾個 child 就要 wait 幾次（Exercise 2 的要求）。
- 用 wait 可以**保證 parent 最後才輸出**。
- （補充）child 結束了但 parent 沒 wait，child 會變成 **zombie**；parent 比 child 先結束，child 變成 **orphan**，會被 init/systemd 收養（PPID 變成 1 或其他 subreaper）。

### 2.6 exec 系列
- fork 只能做出「一模一樣」的 process；若要讓 child 執行**不同的程式**，就用 exec。
- exec 用新的程式**覆蓋（overlay）**目前 process 的 image（text、data、heap、stack 全換掉），**PID 不變**。
- **exec 成功就不會返回**：exec 之後的程式碼不會被執行；只有失敗才會回傳 -1。
- 常見模式 **fork–exec**：parent fork 出 child → child 呼叫 exec 執行新程式 → parent 繼續原本的程式碼（通常還會 wait）。
- `execlp` 名稱解讀：**l = list**（參數一個一個列出來）；**p = path**（會去 `PATH` 環境變數的目錄找程式，所以只寫 `"ls"` 就好，不必寫 `/bin/ls`）。
- 參數慣例：第 1 個參數是「要找的檔案」，第 2 個是新程式的 **argv[0]（程式名稱，通常和第 1 個一樣）**，後面是命令列參數，**最後一定要以 `NULL` 結尾**。

---

## 3. 指令／API／system call 速查表

| 名稱 | 標頭 | 用途 | 回傳值 | 範例 |
|---|---|---|---|---|
| `pid_t fork(void)` | `<unistd.h>` | 建立 child process | parent 得 child PID；child 得 0；失敗 -1 | `pid_t pid = fork();` |
| `pid_t getpid(void)` | `<unistd.h>` | 自己的 PID | PID | `getpid()` |
| `pid_t getppid(void)` | `<unistd.h>` | parent 的 PID | PPID | `getppid()` |
| `pid_t wait(int *stat_loc)` | `<sys/wait.h>` | 等待任一 child 結束 | 結束的 child PID；錯誤 -1 | `wait(NULL);` |
| `int execlp(const char *file, const char *arg0, ..., (char*)0)` | `<unistd.h>` | 用新程式取代目前 process | 成功不返回；失敗 -1 | `execlp("ls", "ls", "-l", NULL);` |
| `exit(int status)` | `<stdlib.h>` | 結束 process | — | `exit(0);` |
| `perror(const char *s)` | `<stdio.h>` | 印出錯誤原因 | — | `perror("fork");` |
| `fflush(stdout)` | `<stdio.h>` | 把 stdout buffer 立刻寫出 | — | fork 前使用 |

---

## 4. 程式範例逐行解說

### 4.1 記憶體區段範例
```c
int a = 10;               // global → data section
void function(){
    int b = 20;           // local → stack section
}
int main(){
    int c = 30;           // local → stack section
    int *d = (int *)malloc(sizeof(int));   // d 本身在 stack，*d 指向 heap
    *d = 40;
    function();
    free(d);
    return 0;
}
```
注意：**指標變數 `d` 本身在 stack**，它指到的那塊 4 bytes 在 heap。

### 4.2 fork + switch
```c
pid_t new_pid = fork();
switch(new_pid){
    case -1:
        printf("fork error\n");
    case 0:
        printf("------------this is child process------------\n");
        printf("Pid : %d\nparent pid : %d\n", getpid(), getppid());
        printf("---------------------------------------------\n");
        break;
    default:
        printf("------------this is parent process-----------\n");
        printf("Pid : %d\nchild pid : %d\n", getpid(), new_pid);
        printf("---------------------------------------------\n");
        break;
}
```
輸出（PID 每次不同）：
```
------------this is child process------------
Pid : 5962
parent pid : 5961
---------------------------------------------
------------this is parent process-----------
Pid : 5961
child pid : 5962
---------------------------------------------
```
- child 的 `getppid()` = parent 的 `getpid()`；parent 的 `new_pid` = child 的 `getpid()`。
- **陷阱**：投影片這版 `case -1:` 後面**沒有 `break`**，fork 失敗時會 fall-through 執行 `case 0` 的內容，印出錯誤的「child」資訊。後面的 wait 範例有補上 break。
- child 和 parent 的區塊誰先印不一定（這次是 child 先）。

### 4.3 fork + if（等價寫法）
```c
pid_t new_pid = fork();
if (new_pid < 0) { printf("fork error\n"); return 0; }
else if (new_pid == 0) { /* child */ printf("Pid : %d\nParent pid : %d\n", getpid(), getppid()); return 0; }
else { /* parent */ printf("Pid : %d\nChild pid : %d\n", getpid(), new_pid); return 0; }
```

### 4.4 多次 fork + 兩次 wait
```c
int main(){
    if (fork() == 0) {
        printf("I am child process 1.\n");
        printf("Pid: %d\tParent Pid: %d\n", getpid(), getppid());
        return 0;                 // child 1 到此結束，不會往下執行第二個 fork
    }
    if (fork() == 0) {
        printf("I am child process 2.\n");
        printf("Pid: %d\tParent Pid: %d\n", getpid(), getppid());
        return 0;
    }
    wait(NULL);
    wait(NULL);                   // parent 有 2 個 child → wait 2 次
    printf("I'm parent process.\n");
    printf("PID: %d\tParent PID: %d\n", getpid(), getppid());
    return 0;
}
```
輸出範例：
```
I am child process 1.
Pid: 6087       Parent Pid: 6086
I am child process 2.
Pid: 6088       Parent Pid: 6086
I'm parent process.
Pid: 6086       Parent Pid: 5746     ← 5746 是 shell 的 PID
```
- **能保證的**：parent 最後印（兩次 wait）。
- **不能保證的**：child 1 與 child 2 的先後（只是程式太短，child 1 通常先跑完）。
- **關鍵**：child 裡的 `return 0` 很重要。如果拿掉，child 1 會繼續往下執行第二個 `fork()`，多生出一個 process。

### 4.5 wait 範例
```c
switch (new_pid) {
    case -1: printf("fork error\n"); break;
    case 0:  printf("\n-------this is child process-------\n");
             printf("pid: %d\t parent pid: %d\n", getpid(), getppid()); break;
    default: wait(NULL);                         // 先等 child 結束
             printf("\n-------this is parent process------\n");
             printf("pid: %d\t child pid: %d\n", getpid(), new_pid); break;
}
```
因為 parent 先 wait，**輸出一定是 child 區塊在前、parent 區塊在後**。

### 4.6 exec 範例
```c
pid_t new_pid = fork();
switch(new_pid){
    case -1: printf("fork error\n"); break;
    case 0 : execlp("/bin/ls", "ls", "-l", NULL);   // child 變成 ls -l
             break;                                 // exec 成功就不會執行到這
    default: printf("\n------this is parent process-------\n");
             printf("Pid : %d\t chile pid : %d\t", getpid(), new_pid);
             printf("\n------------------------------------\n");
             break;
}
```
- child 印出目錄的詳細列表（`ls -l` 的結果），parent 印自己的資訊。
- 投影片的第一個參數寫了完整路徑 `/bin/ls`；因為是 `execlp`，寫 `"ls"` 也可以（會去 PATH 找）。
- 這個範例 **parent 沒有 wait**，所以 `ls` 的輸出和 parent 的輸出順序不保證，甚至可能 parent 先結束、shell prompt 先出現，`ls` 輸出接在後面。

### 4.7 學生作業 practice/Lab6_ex1/ex1.c（Exercise 1）
```c
int global_var = 10;                       // data section
void print_info(const char *title, int local_var) {
    printf("%s\n", title);
    printf("PID: %-8d ,PPID: %-8d , Global variable: %-4d , Local variable: %d\n",
           getpid(), getppid(), global_var, local_var);
}
int main(void) {
    int local_var = 10;                    // stack
    pid_t pid;
    print_info("Initial value:", local_var);
    printf("------...------\n");
    fflush(stdout);                        // fork 前清空 buffer
    pid = fork();
    if (pid < 0) { perror("fork"); exit(1); }
    else if (pid == 0) {                   // child
        print_info("Child process:", local_var);
        global_var++;  local_var++;
        print_info("Child process after increment:", local_var);
        exit(0);
    } else {                               // parent
        wait(NULL);
        print_info("Parent process:", local_var);
        print_info("Parent process after increment:", local_var);
        printf("------...------\n");
    }
    return 0;
}
```
輸出（與投影片 Result 一致）：
```
Initial value:
PID: 6461     ,PPID: 5746     , Global variable: 10   , Local variable: 10
--------------------------------------------------------------------------
Child process:
PID: 6462     ,PPID: 6461     , Global variable: 10   , Local variable: 10
Child process after increment:
PID: 6462     ,PPID: 6461     , Global variable: 11   , Local variable: 11
Parent process:
PID: 6461     ,PPID: 5746     , Global variable: 10   , Local variable: 10
Parent process after increment:
PID: 6461     ,PPID: 5746     , Global variable: 10   , Local variable: 10
--------------------------------------------------------------------------
```
**重點**：child 把 global 和 local 都加到 11，但 parent 看到的**仍是 10**——fork 複製了整份記憶體，**連 global 變數也不共享**（常見誤解：以為 global 會共享）。

**評論**：
- `fflush(stdout)` 很好：避免 stdout 被導向檔案或 pipe 時（全緩衝），fork 把還沒印出的 "Initial value" 複製到 child 造成**重複輸出**（例如 `./ex1 > out.txt` 會看到兩次）。
- `wait(NULL)` 讓 parent 在 child 之後輸出，順序固定、好 demo。
- 「Parent process after increment」那行 parent 其實**沒有做遞增**（只是再印一次），這和投影片的 Result 一致（parent 兩行都是 10）。題目文字說「increases both the values」語意上有點模糊；如果助教要求 parent 也要自己遞增，parent 會印出 11——但那是 parent 自己的 11，與 child 無關。考試時能解釋這一點即可。

### 4.8 學生作業 practice/Lab6_ex2/ex2.c（Exercise 2：process tree）
目標樹：
```
        A
      / | \
     B  D  E
     |
     C
```
```c
pid = fork();                          // A 產生 E
if (pid == 0) { report('E', "Child process"); exit(0); }

pid = fork();                          // A 產生 B
if (pid == 0) {                        // B
    pid_t c = fork();                  // B 產生 C
    if (c == 0) { report('C', "Child process"); exit(0); }
    wait(NULL);                        // B 有 1 個 child → wait 1 次
    report('B', "Child process");
    exit(0);
}

pid = fork();                          // A 產生 D
if (pid == 0) { report('D', "Child process"); exit(0); }

for (i = 0; i < 3; i++) wait(NULL);    // A 有 3 個 child → wait 3 次
report('A', "Parent process");
```
輸出範例（與投影片一致）：
```
Child process E -   PID: 6455     , PPID: 6454
Child process C -   PID: 6457     , PPID: 6456
Child process B -   PID: 6456     , PPID: 6454
Child process D -   PID: 6458     , PPID: 6454
Parent process A -  PID: 6454     , PPID: 5746
```
- 驗證：B、D、E 的 PPID = A 的 PID（6454）；C 的 PPID = B 的 PID（6456）。
- **保證**：C 在 B 之前印（B 先 wait C）；A 最後印（wait 3 次）。E、D、B 之間的相對順序**不保證**。
- 每個 child 在自己的分支最後都 `exit(0)`，**防止 child 繼續往下執行 parent 的 fork**——這正是備註強調的「不能單純連續 fork 四次」。
- 程式正確、完整，符合題目要求。

**為什麼不能連續 fork？** `fork(); fork(); fork();` 會因為每個 child 也繼續執行後面的 fork，產生 2³ = 8 個 process，形狀也不對。

---

## 5. 上機考實作步驟

```bash
mkdir -p ~/lab6 && cd ~/lab6
vim ex1.c
gcc ex1.c -o ex1
./ex1

vim ex2.c
gcc ex2.c -o ex2
./ex2

# 檢查 process tree（補充）：讓程式 sleep 一下，另開 terminal 看
ps -ef --forest | grep ex2
pstree -p <A 的 PID>
```

Exercise 2 的骨架（考試時可直接改）：
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    if (fork() == 0) {                       /* B */
        if (fork() == 0) {                   /* C */
            printf("C: pid=%d ppid=%d\n", getpid(), getppid());
            exit(0);
        }
        wait(NULL);
        printf("B: pid=%d ppid=%d\n", getpid(), getppid());
        exit(0);
    }
    if (fork() == 0) { printf("D: pid=%d ppid=%d\n", getpid(), getppid()); exit(0); }
    if (fork() == 0) { printf("E: pid=%d ppid=%d\n", getpid(), getppid()); exit(0); }
    for (int i = 0; i < 3; i++) wait(NULL);
    printf("A: pid=%d ppid=%d\n", getpid(), getppid());
    return 0;
}
```

---

## 6. 常見錯誤與陷阱

1. **child 分支沒有 `exit()`／`return`** → child 繼續往下跑 parent 的程式碼，又 fork 出多餘的 process。
2. 以為 **global 變數在 fork 後共享** → 錯，每個 process 各有一份複製。
3. 以為 fork 在 child 回傳的 0 是 child 的 PID → 錯，要用 `getpid()`。
4. 以為 parent/child **執行順序固定** → 由 scheduler 決定；要固定順序就用 `wait()`。
5. **wait 次數不足**：有 3 個 child 只 wait 1 次 → parent 可能在其他 child 結束前就印出／結束。
6. `execlp` 參數**忘了最後的 `NULL`** → 未定義行為。
7. 以為 exec 之後的程式碼還會執行 → exec 成功就不會返回；只有失敗才會繼續（應在後面加 `perror("execlp"); exit(1);`）。
8. `switch` 中 `case -1:` 忘了 `break` → fall-through（投影片範例 4.2 就有這個問題）。
9. **printf buffer + fork**：fork 前沒 `fflush(stdout)` 且輸出被導向檔案時，fork 前的輸出會出現兩次。
10. 沒 `#include <sys/wait.h>` 就用 `wait()` → 編譯警告／錯誤。

---

## 7. 筆試模擬題

**Q1（選擇）** `fork()` 在 child process 中的回傳值為？
(A) child 的 PID　(B) parent 的 PID　(C) 0　(D) -1
> **答：C**。parent 拿到 child 的 PID；失敗 -1。

**Q2（選擇）** 下列變數各存在 process 的哪個區段？`int g = 1;`（全域）、函式內 `int x = 2;`、`malloc` 出來的空間
(A) data / stack / heap　(B) stack / data / heap　(C) data / heap / stack　(D) text / stack / heap
> **答：A**。

**Q3（輸出預測）** 下列程式共印出幾行 `hi`？
```c
int main(){ fork(); fork(); printf("hi\n"); return 0; }
```
> **答：4 行**。第一次 fork 後 2 個 process，各自再 fork → 4 個 process，每個印一次。

**Q4（輸出預測）**
```c
int x = 5;
int main(){
    pid_t p = fork();
    if (p == 0) { x += 10; printf("child x=%d\n", x); return 0; }
    wait(NULL);
    printf("parent x=%d\n", x);
    return 0;
}
```
> **答：**
> ```
> child x=15
> parent x=5
> ```
> fork 複製記憶體，child 的修改不影響 parent；wait 保證 child 先印。

**Q5（是非）** exec 成功後，exec 呼叫之後的程式碼仍會繼續執行。
> **答：錯**。process image 已被新程式覆蓋，成功的 exec 不會返回。

**Q6（簡答）** `execlp("ls", "ls", "-l", NULL)` 中各參數的意義？`l` 和 `p` 代表什麼？
> **答：** 第 1 個 `"ls"`：要到 PATH 中尋找的檔案；第 2 個 `"ls"`：新程式的 argv[0]；`"-l"`：命令列參數；`NULL`：參數列結束標記。l = list（參數逐一列出），p = path（自動搜尋 PATH）。

**Q7（選擇）** `wait()` 的回傳值為？
(A) child 的 exit status　(B) 結束的 child 之 PID，錯誤時 -1　(C) 永遠 0　(D) parent 的 PID
> **答：B**。exit status 透過參數 `stat_loc` 取得。

**Q8（簡答）** parent 有 3 個 child，要確保 parent 在所有 child 結束後才印訊息，要怎麼做？
> **答：** 呼叫 3 次 `wait()`（一次 wait 只回收一個 child），例如 `for(i=0;i<3;i++) wait(NULL);`。

**Q9（輸出預測）** 投影片多次 fork 範例中，下列敘述何者「一定」成立？
(A) child 1 一定在 child 2 前印　(B) parent 一定最後印　(C) child 2 一定最後印　(D) 輸出順序每次都一樣
> **答：B**。parent wait 兩次後才印；child 之間的順序由 scheduler 決定。

**Q10（是非）** 一個 process 包含程式碼，也包含 program counter 與 registers 等目前執行狀態。
> **答：對**。

**Q11（簡答）** 為什麼 Exercise 2 不能直接 `fork()` 四次？
> **答：** fork 後 child 也會執行後面的程式碼，連續 fork 4 次會產生 2⁴ = 16 個 process，且樹的形狀不符。必須用 fork 回傳值判斷身分，只讓特定 process 繼續 fork，並讓沒有 child 的 process 印完就 exit。

**Q12（程式填空）** 讓 child 執行 `ls -l`、parent 等 child 結束後印 "done"：
```c
pid_t pid = fork();
if (pid == 0) { ________________; perror("exec"); exit(1); }
________;
printf("done\n");
```
> **答：** `execlp("ls", "ls", "-l", NULL)`；`wait(NULL)`。

**Q13（輸出預測）** 在 child 中呼叫 `getppid()`，若 parent 已經先結束，可能得到什麼？
> **答：**（補充）child 成為 orphan，被 init/systemd（或 subreaper）收養，`getppid()` 會回傳收養者的 PID（例如 1），而不是原 parent 的 PID。
