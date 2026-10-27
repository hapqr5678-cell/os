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
