#include <stdio.h>
#include <errno.h>
#include <sys/syscall.h>
#include <unistd.h>
int main(void) {
    long r;
    r = syscall(462, 3);
    printf("n=3  -> %ld\n", r);
    r = syscall(462, -1);
    printf("n=-1 -> %ld\n", r);
    if (r == -1)
        perror("syscall");
    return 0;
}
