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
