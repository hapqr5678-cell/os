#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/time64.h>
#include <linux/timekeeping.h>

extern time64_t ts;

int init_module(void) {
	time64_t now = ktime_get_real_seconds();
	time64_t diff = now - ts;
	long long years, days, hours, minutes, seconds;

	printk(KERN_INFO "myage insert successfully!\n");

	if (diff < 0) {
		printk(KERN_WARNING "You are not born yet! (diff=%lld)\n", diff);
		return 0;
	}

	seconds = diff % 60;          diff /= 60;    /* 由小單位往大單位拆 */
	minutes = diff % 60;          diff /= 60;
	hours   = diff % 24;          diff /= 24;    /* 這時 diff = 總天數 */
	years   = diff / 365;
	days    = diff % 365;

	printk(KERN_INFO "You have lived %lld years %lld days %lld hours %lld minutes %lld seconds!\n",
	       years, days, hours, minutes, seconds);
	return 0;
}

void cleanup_module(void) {
	printk(KERN_INFO "myage remove successfully!\n");
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Lab5 exam: myage with time");
