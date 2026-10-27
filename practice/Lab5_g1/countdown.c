#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/time64.h>
#include <linux/timekeeping.h>

extern time64_t ts;                          /* 來自 deadline */

int init_module(void) {
	time64_t now = ktime_get_real_seconds(); /* 現在的時間戳記（秒） */
	time64_t diff = ts - now;                /* 注意方向：目標 - 現在 */

	printk(KERN_INFO "countdown insert successfully!\n");

	if (diff < 0) {                          /* 目標日已經過了 */
		diff = -diff;
		printk(KERN_WARNING "The target date has passed %lld days %lld hours ago!\n",
		       diff / 86400, diff % 86400 / 3600);
		return 0;
	}

	printk(KERN_INFO "There are %lld days %lld hours %lld minutes %lld seconds left!\n",
	       diff / 86400, diff % 86400 / 3600, diff % 3600 / 60, diff % 60);
	return 0;
}

void cleanup_module(void) {
	printk(KERN_INFO "countdown remove successfully!\n");
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Lab5 exam: countdown");
