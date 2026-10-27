#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
int main(void) {
    printf("hello: %ld\n", syscall(462));
    printf("add  : %ld\n", syscall(463, 10, 20));
    return 0;
}
