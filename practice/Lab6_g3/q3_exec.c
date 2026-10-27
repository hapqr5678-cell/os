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
