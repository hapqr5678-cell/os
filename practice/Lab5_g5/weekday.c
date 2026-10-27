#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/time64.h>
#include <linux/timekeeping.h>

extern time64_t ts;

int init_module(void) {
	const char *name[] = {"Sunday", "Monday", "Tuesday", "Wednesday",
	                      "Thursday", "Friday", "Saturday"};
	time64_t now = ktime_get_real_seconds();
	long long days = ts / 86400;          /* 1970/1/1 到生日經過幾天（ts >= 0） */
	long long lived;

	printk(KERN_INFO "weekday insert successfully!\n");
	/* 1970/1/1 是星期四(4)，每過一天往後一天 */
	printk(KERN_INFO "You were born on %s.\n", name[(days + 4) % 7]);

	if (now < ts) {
		printk(KERN_WARNING "You are not born yet!\n");
		return 0;
	}
	lived = (now - ts) / 86400;
	printk(KERN_INFO "You have lived %lld days = %lld weeks and %lld days.\n",
	       lived, lived / 7, lived % 7);
	return 0;
}

void cleanup_module(void) {
	printk(KERN_INFO "weekday remove successfully!\n");
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Lab5 exam: weekday");
