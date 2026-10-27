#include <linux/kernel.h>
#include <linux/syscalls.h>

SYSCALL_DEFINE2(hello, int, a, int, b)
{
    int sum = a + b;

    pr_info("%d + %d = %d\n", a, b, sum);
    return sum;
}
