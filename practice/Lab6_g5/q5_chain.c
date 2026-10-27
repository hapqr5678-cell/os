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
