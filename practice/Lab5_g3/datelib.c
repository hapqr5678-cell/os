#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/time64.h>

/* function symbol 1：日期有效回傳 1，無效回傳 0 */
int is_valid_date(int y, int m, int d) {
	int mdays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

	if ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0)
		mdays[1] = 29;
	if (y < 1970 || m < 1 || m > 12 || d < 1 || d > mdays[m - 1])
		return 0;
	return 1;
}

/* function symbol 2：日期 → 時間戳記（秒） */
time64_t date2ts(int y, int m, int d) {
	return mktime64(y, m, d, 0, 0, 0);
}

EXPORT_SYMBOL(is_valid_date);
EXPORT_SYMBOL(date2ts);

int init_module(void) {
	printk(KERN_INFO "datelib insert successfully!\n");
	return 0;
}

void cleanup_module(void) {
	printk(KERN_INFO "datelib remove successfully!\n");
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Lab5 exam: datelib");
