# Lab5_g6

> 來源:review/predict/Lab5_predict.md

### Q6. 單一 module：該年第幾天、是否閏年（可能性：低）

**題目敘述**
撰寫一個 kernel module `dayofyear`，參數 `year month date`。若日期無效，顯示錯誤訊息並終止安裝；否則印出該日期是當年的第幾天、該年是否閏年、今年還剩幾天。

```bash
sudo insmod dayofyear.ko year=2100 month=2 date=29     # 2100 不是閏年
sudo insmod dayofyear.ko year=2024 month=8 date=1
sudo dmesg | tail -3
modinfo dayofyear.ko | grep parm
```
預期輸出：
```
insmod: ERROR: could not insert module dayofyear.ko: Invalid parameters
[...] Invalid argument(s).
[...] dayofyear insert successfully!
[...] 2024/8/1 is day 214 of 2024 (a leap year), 152 days left.
parm:           year:year (int)
parm:           month:month (int)
parm:           date:date (int)
```
其他測資：`2000/2/29` → `day 60 … (a leap year), 306 days left`；`2026/10/28` → `day 301 … (not a leap year), 64 days left`。

**變形自／考哪個觀念**
- hello_module_2（p.19~21，同樣 3 個參數、`modinfo` 看 `MODULE_PARM_DESC`）＋ exercise 的無效日期處理（p.35）。
- 閏年規則（4 的倍數且非 100 的倍數，或 400 的倍數）是 exercise 驗證日期必備的。

**可能性：低**。理由：沒有用到 `EXPORT_SYMBOL` 與兩個 module 的相依，涵蓋的小節太少（違反共通邏輯 3「把所有小節串起來」）；比較可能只是考題中的「第一小題」。

**參考答案** ⚠️ 無法在此環境編譯驗證（已用 mock 在 user space 編譯＋模擬執行通過；第幾天公式窮舉比對，見 §4）

`dayofyear.c`
```c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/time64.h>

int year = 2000, month = 1, date = 1;

module_param(year, int, 0);
MODULE_PARM_DESC(year, "year");
module_param(month, int, 0);
MODULE_PARM_DESC(month, "month");
module_param(date, int, 0);
MODULE_PARM_DESC(date, "date");

int init_module(void) {
	int mdays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	int leap = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
	long long nth, total;

	if (leap)
		mdays[1] = 29;

	if (year < 1970 || month < 1 || month > 12 ||
	    date < 1 || date > mdays[month - 1]) {
		printk(KERN_ERR "Invalid argument(s).\n");
		return -EINVAL;
	}

	/* 同一年 1/1 到今天差幾天，+1 就是第幾天 */
	nth = (mktime64(year, month, date, 0, 0, 0) - mktime64(year, 1, 1, 0, 0, 0)) / 86400 + 1;
	total = leap ? 366 : 365;

	printk(KERN_INFO "dayofyear insert successfully!\n");
	printk(KERN_INFO "%d/%d/%d is day %lld of %d (%s year), %lld days left.\n",
	       year, month, date, nth, year, leap ? "a leap" : "not a leap", total - nth);
	return 0;
}

void cleanup_module(void) {
	printk(KERN_INFO "dayofyear remove successfully!\n");
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Lab5 exam: dayofyear");
```
`Makefile`
```make
obj-m += dayofyear.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```
**操作順序**
```bash
make
modinfo dayofyear.ko
sudo insmod dayofyear.ko year=2100 month=2 date=29
sudo insmod dayofyear.ko year=2024 month=8 date=1
sudo dmesg | tail -3
sudo rmmod dayofyear            # 想換參數重測，一定要先 rmmod
sudo insmod dayofyear.ko year=2000 month=2 date=29
```

**容易踩的坑**
1. 同一個 module 已經裝著時再 insmod → `File exists`；換參數前要先 `rmmod`。
2. 閏年判斷的括號：`(y % 4 == 0 && y % 100 != 0) || y % 400 == 0`，2100 不是閏年、2000 是。
3. 「第幾天」記得 `+1`（1/1 是第 1 天，不是第 0 天）。
4. 只用 `mktime64` 相減就不必自己累加每月天數，比較不容易錯。
