#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
int main(void) {
    long r = syscall(462, 3, 5);
    printf("return = %ld\n", r);
    return 0;
}
