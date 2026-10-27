#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/*
 *          A
 *        /   \
 *       B     C
 *      / \     \
 *     D   E     F
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
