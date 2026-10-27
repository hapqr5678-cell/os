#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/time64.h>

int year = 2026, month = 12, date = 25;      /* 預設：聖誕節 */
time64_t ts = 0;

module_param(year, int, 0);
MODULE_PARM_DESC(year, "target year");
module_param(month, int, 0);
MODULE_PARM_DESC(month, "target month");
module_param(date, int, 0);
MODULE_PARM_DESC(date, "target date");

EXPORT_SYMBOL(ts);

int init_module(void) {
	int mdays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

	if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)
		mdays[1] = 29;                       /* 閏年 2 月 29 天 */

	/* 先確認 month 在 1~12，才能用 mdays[month - 1]（|| 短路求值） */
	if (year < 1970 || month < 1 || month > 12 ||
	    date < 1 || date > mdays[month - 1]) {
		printk(KERN_ERR "Invalid argument(s).\n");
		return -EINVAL;                      /* 回傳負數 → insmod 失敗 */
	}

	ts = mktime64(year, month, date, 0, 0, 0);
	printk(KERN_INFO "deadline insert successfully! %d/%d/%d ts=%lld\n",
	       year, month, date, ts);
	return 0;
}

void cleanup_module(void) {
	printk(KERN_INFO "deadline remove successfully!\n");
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Lab5 exam: deadline");
