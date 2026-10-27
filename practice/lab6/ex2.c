#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/*
 * Process tree:
 *          A
 *        / | \
 *       B  D  E
 *       |
 *       C
 */

void report(char name, const char *role)
{
    printf("%s %c -\tPID: %-8d , PPID: %d\n", role, name, getpid(), getppid());
    fflush(stdout);
}

int main(void)
{
    pid_t pid;
    int i;

    /* A 產生 E */
    pid = fork();
    if (pid < 0) { perror("fork"); exit(1); }
    if (pid == 0) {                         /* E：沒有 child */
        report('E', "Child process");
        exit(0);
    }

    /* A 產生 B */
    pid = fork();
    if (pid < 0) { perror("fork"); exit(1); }
    if (pid == 0) {                         /* B */
        pid_t c = fork();                   /* B 產生 C */
        if (c < 0) { perror("fork"); exit(1); }
        if (c == 0) {                       /* C：沒有 child */
            report('C', "Child process");
            exit(0);
        }
        wait(NULL);                         /* B 有 1 個 child → wait 1 次 */
        report('B', "Child process");
        exit(0);
    }

    /* A 產生 D */
    pid = fork();
    if (pid < 0) { perror("fork"); exit(1); }
    if (pid == 0) {                         /* D：沒有 child */
        report('D', "Child process");
        exit(0);
    }

    /* A 有 3 個 child（B、D、E）→ wait 3 次 */
    for (i = 0; i < 3; i++)
        wait(NULL);

    report('A', "Parent process");
    return 0;
}
