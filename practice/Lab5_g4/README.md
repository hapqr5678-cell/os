# Lab5_g4

> 來源:review/predict/Lab5_predict.md

### Q4. 精確足歲＋下次生日倒數（可能性：中）

**題目敘述**
exercise 的 myage 用「365 天＝1 年」近似，會因閏日而偏差。請改寫：
- `date2timestamp`：同原題（參數 year/month/date、export `ts`），但**另外把 `year`、`month`、`date` 也 export**。
- `birthday`：算出真正的「足歲」與「距離上次生日幾天」，以及「下次生日還有幾天幾小時」。ts 在未來時要提示。只能用 `mktime64`。

```bash
sudo insmod date2timestamp.ko year=2001 month=8 date=1
sudo insmod birthday.ko
sudo dmesg | tail -3
```
預期輸出：
```
[...] birthday insert successfully!
[...] You are 25 years old, 88 days since your last birthday.
[...] Your next birthday is in 276 days 22 hours.
```
對照：原本 myage（365 天近似）會印 `25 years and 94 days`，差的 6 天就是 2004、2008、2012、2016、2020、2024 這 6 個閏日。
其他測資：`2005/10/29` → `20 years old, 364 days since…`、`next birthday is in 0 days 22 hours`；`2004/2/29` → `22 years old, 241 days`、`next … 123 days 22 hours`（平年把 3/1 當生日）。

**變形自／考哪個觀念**
- exercise p.32 的「轉成年跟日」——驗證你知道 365 近似會錯在哪（共通邏輯 2：驗證一個觀念）。
- p.23「`int year = 2023;` → variable symbol」：**module parameter 本身也是普通全域變數**，一樣可以 `EXPORT_SYMBOL`。
- `mktime64` 第二次用法：用「生日年份 + age」去算每一年的生日。

**可能性：中**。理由：完全在範圍內，而且直接對應你自己做的 `is_adult()`（18 歲生日 = `mktime64(year + 18, month, date, ...)`，本題是它的一般化）；但需要一點迴圈想法，助教出成必做題的機率比 Q1、Q2 低。

**參考答案** ⚠️ 無法在此環境編譯驗證（已用 mock 在 user space 編譯＋模擬執行通過；足歲迴圈另做窮舉比對，見 §4）

`date2timestamp.c`
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
EXPORT_SYMBOL(year);       /* 參數變數本身也是 variable symbol，一樣可以 export */
EXPORT_SYMBOL(month);
EXPORT_SYMBOL(date);

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
MODULE_DESCRIPTION("Lab5 exam: date2timestamp (export y/m/d)");
```
`birthday.c`
```c
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
```
`Makefile`
```make
obj-m += date2timestamp.o
obj-m += birthday.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```
**操作順序**
```bash
make
sudo insmod date2timestamp.ko year=2001 month=8 date=1
sudo insmod birthday.ko
sudo dmesg | tail -3
sudo rmmod birthday
sudo rmmod date2timestamp
```

**容易踩的坑**
1. B 的 `extern int year, month, date;` 型別要跟 A 一樣是 `int`；B 自己**不要**再宣告同名的 module_param。
2. 如果先前裝的另一個 module 也 export 了 `ts`／`year`，會出現 `exports duplicate symbol` 而裝不起來 → 換題 demo 前先 `lsmod` 確認舊的都 `rmmod` 掉。
3. 2/29 生日：`mktime64(2026, 2, 29, …)` 不會報錯，會自動進位成 3/1（已驗證），所以平年以 3/1 當生日；被問到要說得出來。
4. 迴圈條件是 `<=`：生日當天 00:00（UTC）之後就算滿歲。
5. 不要用 `diff / 365 / 86400` 的近似來「足歲」，題目就是要你避開它。
