#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/timekeeping.h>

extern time64_t ts;
extern int is_adult(time64_t now);

int init_module(void) {
	time64_t now = ktime_get_real_seconds();   /* 當前時間戳記（秒） */
	time64_t diff = now - ts;
	long long days, years;

	printk(KERN_INFO "myage insert successfully!\n");

	if (diff < 0) {
		printk(KERN_WARNING "Your birthday is in the future! (diff=%lld)\n", diff);
		return 0;
	}

	days  = diff / 86400;     /* 1 天 = 86400 秒 */
	years = days / 365;
	days  = days % 365;

	printk(KERN_INFO "Wow you have survived on this world %lld years and %lld days!\n",
	       years, days);

	/* 呼叫 date2timestamp 匯出的函式 */
	if (is_adult(now))
		printk(KERN_INFO "You are an adult (18+).\n");
	else
		printk(KERN_INFO "You are NOT an adult yet (under 18).\n");
	return 0;
}

void cleanup_module(void) {
	printk(KERN_INFO "myage remove successfully!\n");
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Lab5: myage");
