#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
int main(void) {
    printf("%ld\n", syscall(462, 3));
    return 0;
}
