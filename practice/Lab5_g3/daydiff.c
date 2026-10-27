#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/time64.h>

extern int is_valid_date(int y, int m, int d);   /* 來自 datelib */
extern time64_t date2ts(int y, int m, int d);

int year1 = 2001, month1 = 8, date1 = 1;
int year2 = 2026, month2 = 10, date2 = 28;

module_param(year1, int, 0);
MODULE_PARM_DESC(year1, "year of date 1");
module_param(month1, int, 0);
MODULE_PARM_DESC(month1, "month of date 1");
module_param(date1, int, 0);
MODULE_PARM_DESC(date1, "date of date 1");
module_param(year2, int, 0);
MODULE_PARM_DESC(year2, "year of date 2");
module_param(month2, int, 0);
MODULE_PARM_DESC(month2, "month of date 2");
module_param(date2, int, 0);
MODULE_PARM_DESC(date2, "date of date 2");

int init_module(void) {
	time64_t diff;

	if (!is_valid_date(year1, month1, date1) ||
	    !is_valid_date(year2, month2, date2)) {
		printk(KERN_ERR "Invalid argument(s).\n");
		return -EINVAL;
	}

	diff = date2ts(year2, month2, date2) - date2ts(year1, month1, date1);
	printk(KERN_INFO "daydiff insert successfully!\n");
	printk(KERN_INFO "From %d/%d/%d to %d/%d/%d: %lld days\n",
	       year1, month1, date1, year2, month2, date2, diff / 86400);
	return 0;
}

void cleanup_module(void) {
	printk(KERN_INFO "daydiff remove successfully!\n");
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Lab5 exam: daydiff");
