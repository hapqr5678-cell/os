#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int global_var = 10;                 /* 全域變數 */

void print_info(const char *title, int local_var)
{
    printf("%s\n", title);
    printf("PID: %-8d ,PPID: %-8d , Global variable: %-4d , Local variable: %d\n",
           getpid(), getppid(), global_var, local_var);
}

int main(void)
{
    int local_var = 10;              /* 區域變數 */
    pid_t pid;

    print_info("Initial value:", local_var);
    printf("----------------------------------------------------------------------------------\n");
    fflush(stdout);                  /* 先清空 buffer，避免 fork 後重複輸出 */

    pid = fork();
    if (pid < 0) {                   /* fork 失敗 */
        perror("fork");
        exit(1);
    } else if (pid == 0) {           /* Child process */
        print_info("Child process:", local_var);
        global_var++;                /* 只改到 child 自己的 memory image */
        local_var++;
        print_info("Child process after increment:", local_var);
        exit(0);
    } else {                         /* Parent process */
        wait(NULL);                  /* 等 child 結束再印 */
        print_info("Parent process:", local_var);
        /* child 的遞增不會影響 parent，所以值仍是 10 */
        print_info("Parent process after increment:", local_var);
        printf("----------------------------------------------------------------------------------\n");
    }
    return 0;
}
