#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/time64.h>

int year = 2000, month = 1, date = 1;
int hour = 0, minute = 0, second = 0;        /* 新增的三個參數 */
time64_t ts = 0;

module_param(year, int, 0);
MODULE_PARM_DESC(year, "birth year");
module_param(month, int, 0);
MODULE_PARM_DESC(month, "birth month");
module_param(date, int, 0);
MODULE_PARM_DESC(date, "birth date");
module_param(hour, int, 0);
MODULE_PARM_DESC(hour, "birth hour (0-23)");
module_param(minute, int, 0);
MODULE_PARM_DESC(minute, "birth minute (0-59)");
module_param(second, int, 0);
MODULE_PARM_DESC(second, "birth second (0-59)");

EXPORT_SYMBOL(ts);

int init_module(void) {
	int mdays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

	if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)
		mdays[1] = 29;

	if (year < 1970 || month < 1 || month > 12 ||
	    date < 1 || date > mdays[month - 1] ||
	    hour < 0 || hour > 23 || minute < 0 || minute > 59 ||
	    second < 0 || second > 59) {
		printk(KERN_ERR "Invalid argument(s).\n");
		return -EINVAL;
	}

	ts = mktime64(year, month, date, hour, minute, second);
	printk(KERN_INFO "date2timestamp insert successfully! ts=%lld\n", ts);
	return 0;
}

void cleanup_module(void) {
	printk(KERN_INFO "date2timestamp remove successfully!\n");
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Lab5 exam: date2timestamp with time");
