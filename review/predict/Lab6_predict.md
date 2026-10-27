# Lab 6 上機考猜題：Process Control

> 範圍：Lab6.pptx 全部 29 頁，含投影片圖片裡的範例程式與 Result 截圖，以及 practice/Lab6_ex1/ex1.c、ex2.c。
> 每一題的參考答案都已在 Linux 上用 `gcc -Wall` 編譯執行，沒有任何 warning。process 樹題另外用腳本比對每個 process 的 PPID 是否等於它 parent 的 PID。
> 驗證用的原始碼放在 `/tmp/claude-0/-home-user-os/1dcd3112-7322-5920-934d-1535c60cff9f/scratchpad/lab6/`。

---

## 0. 簡報裡實際教了什麼（含圖片內容）

| 頁 | 內容 | 圖片裡的程式／截圖重點 |
|---|---|---|
| p3–p5 | process 四個區段：text / data / heap / stack | p5 範例：`int a = 10;`（global → **data**）、`int b = 20;`（函式內 → **stack**）、`int c = 30;`（main 內 → stack）、`int *d = malloc(...)`（**heap**） |
| p6 | process tree | 課本的 Sched → init → … 樹 |
| p8–p11 | `fork()`：複製 parent 的 memory image；parent 拿到 child PID、child 拿到 0、失敗 -1 | p11 流程圖：「Returns a new PID / Returns zero」 |
| p9 | `getpid()` / `getppid()` | — |
| p12 | Example (switch) | `switch(new_pid){ case -1 / case 0 / default }`，parent 印 `child pid : new_pid` |
| p13 | Example (if) | `if (new_pid < 0) … else if (new_pid == 0) … else …` |
| p14 | `wait()`：parent 停住直到 child 結束；回傳該 child 的 PID，錯誤回傳 -1 | — |
| **p15** | **fork() multiple times** | `if(fork()==0){…; return 0;}` 連寫兩次，接著 `wait(NULL); wait(NULL);`，最後 parent 才印。**有幾個 child 就 wait 幾次**的原型 |
| **p16** | **Parent always waits for child and executes last** | switch 版本，`default:` 裡先 `wait(NULL)` 才印 parent |
| p17–p19 | exec 家族、fork–exec 組合、`execlp(file, arg0, …, (char*)0)` | p18 圖示：child「Load another image」 |
| **p20** | **exec Example** | `case 0: execlp("/bin/ls", "ls", "-l", NULL);`，parent 印 `Pid : %d  chile pid : %d`。**parent 沒有 wait**，截圖剛好是 ls 的輸出先出現 |
| p22–p24 | Exercise 1 | 見第 2 節截圖比對 |
| p25–p27 | Exercise 2 | 樹：A → B、D、E；B → C（圖片是白線透明 PNG，要疊在深色底上才看得到） |

---

## 1. Lab 6 專屬出題邏輯

在「Exercise = 換題目再用一次核心機制」這個共通規則之外，這個 lab 還有幾點自己的特色。

1. **兩個 exercise 剛好對應兩條主線。**
   - Ex1 驗證觀念：fork 之後 memory image 各自獨立（p8、p10、p3–p5）。
   - Ex2 練習結構：用 fork 加 wait 建出指定形狀的 process tree（p6、p14–p16）。
   - 最容易出的變形就是**換一棵樹**，或是**換一組要觀察的變數**。
2. **輸出格式固定是「PID + PPID」。** 兩個 exercise 都要求印 `getpid()`、`getppid()`，助教用 PPID 來判斷樹的形狀對不對。考試時一定要印這兩個值，最好也標上 process 名稱（A、B…）。
3. **wait 的次數 = child 的數目**（p26 明確規定），而且 parent 要最後才印（p16）。所以輸出一定是「後序」：child 都先印，parent 最後印。
4. **exec 有教、有範例（p17–p20），但 exercise 沒考。** 依「範圍僅限簡報、Exercise 換題目」的推論，**fork–exec 是最可能補上的變形點**。p20 的範例甚至沒有 wait，考試很可能就是要你把 Ex1/Ex2 的 wait 觀念和 p20 合起來：「child 用 execlp 跑 ls，parent wait 完再印訊息」。
5. **p3–p5 的四個區段**（data/stack/heap）有範例程式，但 Ex1 只用到 data 跟 stack。heap（malloc）是很自然的延伸。
6. **p15 的「fork 多次」只示範了兩個 child 加兩次 wait。** 把它改成用迴圈 fork N 個，或是問「fork 連續呼叫 n 次會有幾個 process」，都還在簡報範圍內。
7. **簡報沒教、不會考的東西**：`waitpid`、`WEXITSTATUS`、`execv/execvp/execle`、pipe、signal、zombie 的處理。寫答案時不需要用到這些，用 `wait(NULL)` 加上 `execlp` 就夠了。

---

## 2. Ex1 參考答案 vs Result 截圖比對（結論）

**題目文字（p23）**：「Then, increases both the values of the global variable and local variable. Finally, **both processes** print the values…」

**Result 截圖（p24）原文：**
```
Initial value:
PID: 6461   ,PPID: 5746   , Global variable: 10   , Local variable: 10
------------------------------------------------------------------------
Child process:
PID: 6462   ,PPID: 6461   , Global variable: 10   , Local variable: 10
Child process after increment:
PID: 6462   ,PPID: 6461   , Global variable: 11   , Local variable: 11
Parent process:
PID: 6461   ,PPID: 5746   , Global variable: 10   , Local variable: 10
Parent process after increment:
PID: 6461   ,PPID: 5746   , Global variable: 10   , Local variable: 10
------------------------------------------------------------------------
```

**結論：截圖跟 ex1.c 的寫法一致，也就是只有 child 遞增。**
- 截圖中 parent 的「after increment」仍然是 **10 / 10**。如果 parent 也做了 `++`，這裡會是 11，所以截圖裡的 parent 並沒有遞增。截圖要表達的是「child 改了變數，parent 看到的值仍然是 10，沒有被影響」。
- 「both processes」修飾的應該是 **print**（兩個 process 都要印），不是 increase。
- 截圖裡 child 的兩段完整印完之後 parent 才印，表示 parent 有先 `wait()`。ex1.c 也是這樣寫。
- ✅ 已驗證：編譯執行 `practice/Lab6_ex1/ex1.c`，輸出格式和數值與截圖逐行相同（child 11/11，parent 10/10）。
- ⚠️ **考試建議**：題目文字寫什麼就照做。如果考題明確寫「parent 也要遞增」，那 parent 會印 **11 而不是 12**，因為它從自己那份 10 開始加。這正是第 2 題要練的情況。

---

## 3. 猜題（依可能性排序）

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

---

### 第 2 題：Ex1 改成 parent 和 child 都遞增，加上 heap 變數【可能性：高】

**題目**：宣告一個 global 變數、一個 local 變數，以及一個用 `malloc` 配置的 heap 變數，初值都是 10，先印一次（附 PID、PPID）。fork 後：
- child 先印一次，然後三個變數各 **+1**，再印一次。
- parent 用 `wait()` 等 child 結束，印一次，然後三個變數各 **+5**，再印一次。
- （加分）兩個 process 都印出三個變數的位址。

輸出範例：
```
Initial value:
PID: 2009     ,PPID: 1985     , Global: 10   , Local: 10   , Heap: 10
------------------------------------------------------------------
Child process:
PID: 2010     ,PPID: 2009     , Global: 10   , Local: 10   , Heap: 10
Child process after increment:
PID: 2010     ,PPID: 2009     , Global: 11   , Local: 11   , Heap: 11
Child  &global=0x55cbe1fdf010  &local=0x7fffdc8a4528  heap=0x55cbfa0352a0
Parent process:
PID: 2009     ,PPID: 1985     , Global: 10   , Local: 10   , Heap: 10
Parent process after increment:
PID: 2009     ,PPID: 1985     , Global: 15   , Local: 15   , Heap: 15
Parent &global=0x55cbe1fdf010  &local=0x7fffdc8a4528  heap=0x55cbfa0352a0
------------------------------------------------------------------
```

**變形自**：Exercise 1（p22–p24），再加上 p3–p5 的 heap 區段（p5 範例的 `int *d = malloc(sizeof(int))`）。考的觀念是 p8、p10：「fork 複製 memory image」，所以 data、stack、heap 三種區段都各自獨立。
**可能性高的理由**：Ex1 佔 40 分，而且題目本身有「both processes」的歧義（見第 2 節）。改成「兩邊都遞增，而且加的數字不同」可以直接檢查學生是否真的理解：parent 印出來應該是 15，不是 16。加入 heap 只用到 p5 教過的 malloc，沒有超出範圍。
**重點觀念**：parent 和 child 印出的**位址完全相同，值卻不同**。因為 fork 複製的是整個虛擬位址空間，同一個虛擬位址在兩個 process 中對應到不同的實體記憶體。

**參考答案**
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int global_var = 10;                         /* data section */

void print_info(const char *title, int local_var, int *heap_var)
{
    printf("%s\n", title);
    printf("PID: %-8d ,PPID: %-8d , Global: %-4d , Local: %-4d , Heap: %d\n",
           getpid(), getppid(), global_var, local_var, *heap_var);
}

int main(void)
{
    int local_var = 10;                      /* stack section */
    int *heap_var = (int *)malloc(sizeof(int)); /* heap section */
    pid_t pid;

    *heap_var = 10;
    print_info("Initial value:", local_var, heap_var);
    printf("------------------------------------------------------------------\n");
    fflush(stdout);                          /* fork 前一定要清 buffer */

    pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {                   /* child：+1 */
        print_info("Child process:", local_var, heap_var);
        global_var++;
        local_var++;
        (*heap_var)++;
        print_info("Child process after increment:", local_var, heap_var);
        printf("Child  &global=%p  &local=%p  heap=%p\n",
               (void *)&global_var, (void *)&local_var, (void *)heap_var);
        free(heap_var);
        exit(0);
    } else {                                 /* parent：等 child 後 +5 */
        wait(NULL);
        print_info("Parent process:", local_var, heap_var);
        global_var += 5;
        local_var += 5;
        *heap_var += 5;
        print_info("Parent process after increment:", local_var, heap_var);
        printf("Parent &global=%p  &local=%p  heap=%p\n",
               (void *)&global_var, (void *)&local_var, (void *)heap_var);
        printf("------------------------------------------------------------------\n");
        free(heap_var);
    }
    return 0;
}
```

✅ 已驗證：輸出就是上面的範例。child 是 11/11/11，parent 從 10 開始加，結果是 15/15/15。兩邊的三個位址完全相同。透過管線執行（`./q2 | cat`）時，"Initial value" 只出現 1 次。

**容易踩的坑**
- **fork 前沒有 `fflush(stdout)`**：輸出到終端機時 stdout 是 line-buffered，看不出問題。但只要導向檔案或管線（助教常用 `./a.out > out.txt` 收作業），buffer 裡還沒寫出的 "Initial value" 會被複製到 child，**印兩次**。✅ 已驗證：拿掉 fflush 後執行 `./a.out | grep -c "Initial value"`，結果是 `2`。
- 以為 child 改完、parent 再加 5 會得到 16。錯，parent 的值是自己那份 10 + 5 = 15。
- `(*heap_var)++` 不能寫成 `*heap_var++`。後者是讓指標往後移一格，而不是把值加 1。
- 印位址要用 `%p` 並轉型成 `(void *)`，否則 `-Wall` 會出現 warning。
- parent 沒有 wait：child 和 parent 的輸出可能交錯，跟截圖的順序不一樣。

---

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

---

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

---

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

---

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

---

## 4. 考前 5 分鐘速查

```c
#include <stdio.h>      // printf, perror, fflush
#include <stdlib.h>     // exit, malloc, free
#include <unistd.h>     // fork, getpid, getppid, execlp
#include <sys/types.h>  // pid_t
#include <sys/wait.h>   // wait
```

| 項目 | 記住這個 |
|---|---|
| fork 回傳值 | parent 得到 **child PID (>0)**，child 得到 **0**，失敗得到 **-1** |
| fork 之後 | 兩個 process **都從 fork 的下一行**繼續執行；memory image（data/stack/heap）**各一份**，互不影響；**虛擬位址相同，值不同** |
| 誰先跑 | 不確定，由 scheduler 決定。要固定順序就用 `wait` |
| wait | `wait(NULL)` 會卡住直到**任一** child 結束，回傳該 child 的 PID；沒有 child 時回傳 **-1** |
| wait 次數 | **child 數 = wait 次數**（p26），不知道有幾個時用 `while (wait(NULL) > 0);` |
| parent 最後印 | 先 wait 完，再 printf（p16） |
| execlp | `execlp("ls", "ls", "-l", NULL);`：程式名稱、argv[0]、參數…、**NULL 結尾** |
| exec 之後 | 成功就**不會回來**，PID 不變，後面寫 `perror("execlp"); exit(1);` |
| child 區塊 | **最後一定要 `exit(0)`**，否則會跑去執行 parent 的程式碼或繼續迴圈 |
| fflush | **fork 前、exec 前都要 `fflush(stdout)`**，否則導向檔案時會重複輸出或遺失 |
| n 次 fork | 總共 2ⁿ 個 process |
| 區段 | global 放在 **data**、函式內的區域變數放在 **stack**、malloc 配置的放在 **heap**、程式碼放在 **text** |
| Ex1 截圖 | parent 的 after increment **仍然是 10**，只有 child 遞增。若題目要求 parent 也加，就從 10 開始加 |
| Ex2 樹 | A→{B,D,E}，B→C。A wait 3 次，B wait 1 次 |

**畫樹寫程式三步驟**：① 每個節點標上 child 數，也就是 wait 次數；② 每個 fork 後面接 `if (pid==0) { …; exit(0); }`；③ 葉節點直接印完 exit，中間節點 fork → wait → 印 → exit。

**輸出格式模板**（照截圖）：
```c
printf("Child process %c -\tPID: %-8d , PPID: %d\n", name, getpid(), getppid());
printf("PID: %-8d ,PPID: %-8d , Global variable: %-4d , Local variable: %d\n", ...);
```
編譯：`gcc -Wall -o ex ex.c && ./ex`。要檢查 fflush 有沒有寫對：`./ex | cat`，看輸出有沒有重複。
