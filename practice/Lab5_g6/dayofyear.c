#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/time64.h>

int year = 2000, month = 1, date = 1;

module_param(year, int, 0);
MODULE_PARM_DESC(year, "year");
module_param(month, int, 0);
MODULE_PARM_DESC(month, "month");
module_param(date, int, 0);
MODULE_PARM_DESC(date, "date");

int init_module(void) {
	int mdays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	int leap = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
	long long nth, total;

	if (leap)
		mdays[1] = 29;

	if (year < 1970 || month < 1 || month > 12 ||
	    date < 1 || date > mdays[month - 1]) {
		printk(KERN_ERR "Invalid argument(s).\n");
		return -EINVAL;
	}

	/* 同一年 1/1 到今天差幾天，+1 就是第幾天 */
	nth = (mktime64(year, month, date, 0, 0, 0) - mktime64(year, 1, 1, 0, 0, 0)) / 86400 + 1;
	total = leap ? 366 : 365;

	printk(KERN_INFO "dayofyear insert successfully!\n");
	printk(KERN_INFO "%d/%d/%d is day %lld of %d (%s year), %lld days left.\n",
	       year, month, date, nth, year, leap ? "a leap" : "not a leap", total - nth);
	return 0;
}

void cleanup_module(void) {
	printk(KERN_INFO "dayofyear remove successfully!\n");
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Lab5 exam: dayofyear");
