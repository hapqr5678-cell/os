# Lab6_g2

> 來源:review/predict/Lab6_predict.md

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
