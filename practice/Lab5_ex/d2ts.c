#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/time64.h>
#include <linux/time.h>

int year = 2000, month = 1, date = 1;
time64_t ts = 0;

module_param(year, int, 0);
MODULE_PARM_DESC(year, "birth year");
module_param(month, int, 0);
MODULE_PARM_DESC(month, "birth month");
module_param(date, int, 0);
MODULE_PARM_DESC(date, "birth date");

EXPORT_SYMBOL(ts);

/* 判斷在 now 這個時間點是否已滿 18 歲：是回傳 1，否回傳 0 */
int is_adult(time64_t now) {
	time64_t adult_ts = mktime64(year + 18, month, date, 0, 0, 0);  /* 18 歲生日 */
	return now >= adult_ts;
}
EXPORT_SYMBOL(is_adult);

int init_module(void) {
	int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	int leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);

	if (leap)
		days_in_month[1] = 29;

	/* 檢查日期是否有效 */
	if (year < 1970 || month < 1 || month > 12 ||
	    date < 1 || date > days_in_month[month - 1]) {
		printk(KERN_ERR "Invalid argument(s).\n");
		return -EINVAL;   /* 回傳負數以中斷安裝 */
	}

	ts = mktime64(year, month, date, 0, 0, 0);
	printk(KERN_INFO "date2timestamp insert successfully! ts=%lld\n", ts);
	return 0;
}

void cleanup_module(void) {
	printk(KERN_INFO "date2timestamp remove successfully!\n");
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Lab5: date2timestamp");
