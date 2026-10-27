#include <linux/kernel.h>
#include <linux/syscalls.h>

SYSCALL_DEFINE0(hello)
{
    pr_info("Hello\n");
    return 0;
}
