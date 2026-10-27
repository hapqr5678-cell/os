#include <linux/kernel.h>
#include <linux/syscalls.h>

SYSCALL_DEFINE1(hello, int, n)
{
    int i;
    long sum = 0;

    for (i = 1; i <= n; i++) {
        sum += i;
        printk(KERN_INFO "i=%d sum=%ld\n", i, sum);   /* KERN_INFO 後面「沒有逗號」 */
    }
    return sum;
}
