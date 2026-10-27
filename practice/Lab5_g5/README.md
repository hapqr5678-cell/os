# Lab5_g5

> 來源:review/predict/Lab5_predict.md

### Q5. 出生那天是星期幾（可能性：中低）

**題目敘述**
- `date2timestamp`：同原題。
- `weekday`：用 `ts` 算出生日是星期幾（已知 1970/1/1 是星期四），並印出活了幾天、合幾週又幾天。

```bash
sudo insmod date2timestamp.ko year=2001 month=8 date=1
sudo insmod weekday.ko
sudo dmesg | tail -3
```
預期輸出：
```
[...] weekday insert successfully!
[...] You were born on Wednesday.
[...] You have lived 9219 days = 1317 weeks and 0 days.
```
`year=1970 month=1 date=1` → `Thursday`、`20754 days = 2964 weeks and 6 days`。（今天 2026/10/28 是星期三，和 2001/8/1 同星期，所以剛好整週。）

**變形自／考哪個觀念**
- 只換 B 的計算（共通邏輯 1）；考你懂 timestamp 是「從 1970/1/1 00:00 UTC 起算的秒數」（p.32、p.35）。
- 字串陣列＋整數 `%`，沒有新 API。

**可能性：中低**。理由：需要知道「1970/1/1 是星期四」這個簡報沒給的常識，題目大概會直接告訴你；屬於加分／延伸題型。

**參考答案** ⚠️ 無法在此環境編譯驗證（已用 mock 在 user space 編譯＋模擬執行通過；星期公式窮舉比對 1970~2200 年每一天，見 §4）

`date2timestamp.c`（就是原題答案）
```c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/time64.h>

int year = 2000, month = 1, date = 1;
time64_t ts = 0;

module_param(year, int, 0);
MODULE_PARM_DESC(year, "birth year");
module_param(month, int, 0);
MODULE_PARM_DESC(month, "birth month");
module_param(date, int, 0);
MODULE_PARM_DESC(date, "birth date");

EXPORT_SYMBOL(ts);

int init_module(void) {
	int mdays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

	if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)
		mdays[1] = 29;

	if (year < 1970 || month < 1 || month > 12 ||
	    date < 1 || date > mdays[month - 1]) {
		printk(KERN_ERR "Invalid argument(s).\n");
		return -EINVAL;
	}

	ts = mktime64(year, month, date, 0, 0, 0);
	printk(KERN_INFO "date2timestamp insert successfully! ts=%lld\n", ts);
	return 0;
}

void cleanup_module(void) {
	printk(KERN_INFO "date2timestamp remove successfully!\n");
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Lab5 exam: date2timestamp");
```
`weekday.c`
```c
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
```
`Makefile`
```make
obj-m += date2timestamp.o
obj-m += weekday.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```
**操作順序**：同 Q4（`date2timestamp` → `weekday`，拆的時候反過來）。

**容易踩的坑**
1. 公式是 `(天數 + 4) % 7`，0＝星期日；`+4` 是因為 1970/1/1 是星期四。
2. 若允許 1970 以前（ts < 0）：C 的 `/`、`%` 向 0 取整，`-1 / 86400 = 0`、`-1 % 7 = -1`，會算錯甚至陣列越界。本答案沿用 `year < 1970` 視為無效，所以 ts ≥ 0。
3. 若要算「今天」星期幾，`now` 是 UTC，台灣 00:00~08:00 之間會差一天，要先 `+ 8 * 3600`。
4. `const char *name[]` 放在函式裡就好，不需要 export。
