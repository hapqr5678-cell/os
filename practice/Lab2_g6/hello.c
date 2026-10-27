#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/errno.h>        /* EINVAL;其實 syscalls.h 已間接引入,寫上較保險 */

SYSCALL_DEFINE1(hello, int, n)
{
    int i;

    if (n <= 0 || n > 10)
        return -EINVAL;         /* kernel 回傳「負的錯誤碼」 */
    for (i = 0; i < n; i++)
        pr_info("hello\n");
    return 0;
}
