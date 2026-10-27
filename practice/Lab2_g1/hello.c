#include <linux/kernel.h>
#include <linux/syscalls.h>

SYSCALL_DEFINE1(hello, int, n)
{
    int i;

    for (i = 0; i < n; i++)
        pr_info("hello\n");      /* 等同 printk(KERN_INFO "hello\n"); */
    return 0;
}
