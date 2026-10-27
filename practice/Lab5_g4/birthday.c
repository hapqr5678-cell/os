#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/time64.h>
#include <linux/timekeeping.h>

extern time64_t ts;
extern int year, month, date;

int init_module(void) {
	time64_t now = ktime_get_real_seconds();
	time64_t last, next;
	int age = 0;

	printk(KERN_INFO "birthday insert successfully!\n");

	if (now < ts) {
		printk(KERN_WARNING "You are not born yet!\n");
		return 0;
	}

	/* 足歲：只要「第 age+1 次生日」已經到了，就再加一歲 */
	while (mktime64(year + age + 1, month, date, 0, 0, 0) <= now)
		age++;

	last = mktime64(year + age, month, date, 0, 0, 0);       /* 最近一次生日 */
	next = mktime64(year + age + 1, month, date, 0, 0, 0);   /* 下一次生日 */

	printk(KERN_INFO "You are %d years old, %lld days since your last birthday.\n",
	       age, (now - last) / 86400);
	printk(KERN_INFO "Your next birthday is in %lld days %lld hours.\n",
	       (next - now) / 86400, (next - now) % 86400 / 3600);
	return 0;
}

void cleanup_module(void) {
	printk(KERN_INFO "birthday remove successfully!\n");
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Lab5 exam: birthday");
