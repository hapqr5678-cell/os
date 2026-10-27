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
