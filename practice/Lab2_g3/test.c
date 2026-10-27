#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
int main(void) {
    int n = 5;
    long r = syscall(462, n);
    printf("sum(1..%d) = %ld\n", n, r);
    return 0;
}
