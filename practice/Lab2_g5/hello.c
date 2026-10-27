#include <linux/kernel.h>
#include <linux/syscalls.h>

SYSCALL_DEFINE0(hello)
{
    pr_info("Hello\n");
    return 0;
}

SYSCALL_DEFINE2(add, int, a, int, b)
{
    pr_info("add: %d + %d = %d\n", a, b, a + b);
    return a + b;
}
