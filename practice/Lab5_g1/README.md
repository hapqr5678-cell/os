# Lab5_g1

> 來源:review/predict/Lab5_predict.md

### Q1. 倒數計時（可能性：高）

**題目敘述**
請撰寫兩個 kernel module：
- `deadline`：module parameters `year`、`month`、`date`（預設 2026/12/25），export symbol `ts`。印出該日期的時間戳記；若日期無效，印出錯誤訊息並終止安裝。
- `countdown`：取得當前時間戳記，算出**距離 deadline 還剩幾天幾時幾分幾秒**；若該日期已經過了，要提示「已經過了幾天」。

```bash
sudo insmod deadline.ko year=2026 month=2 date=29     # 2026 不是閏年 → 失敗
sudo insmod deadline.ko year=2026 month=12 date=25
sudo insmod countdown.ko
sudo dmesg | tail -4
```
預期輸出：
```
insmod: ERROR: could not insert module deadline.ko: Invalid parameters
[...] Invalid argument(s).
[...] deadline insert successfully! 2026/12/25 ts=1798156800
[...] countdown insert successfully!
[...] There are 57 days 22 hours 0 minutes 0 seconds left!
```
日期已過（`year=2026 month=10 date=1`）時：`The target date has passed 27 days 2 hours ago!`

**變形自／考哪個觀念**
- exercise p.31~34 的結構完全不變，只把 `now - ts` 改成 `ts - now`（考你是否真的懂 B 在算什麼，而不是背答案）。
- 參數＋MODULE_PARM_DESC（p.17~20）、EXPORT_SYMBOL／extern（p.23~26）、安裝順序（p.28~29）、`-EINVAL`（p.35）、「小於 0 要提示」（p.34）。

**可能性：高**。理由：只改一個減法方向和輸出單位，助教出題成本最低、又能區分「理解」和「背答案」；「計算到某日期還有幾天」是最直覺的時間戳應用。

**參考答案** ⚠️ 無法在此環境編譯驗證（已用 mock 在 user space 編譯＋模擬執行通過）

`deadline.c`
```c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/time64.h>

int year = 2026, month = 12, date = 25;      /* 預設：聖誕節 */
time64_t ts = 0;

module_param(year, int, 0);
MODULE_PARM_DESC(year, "target year");
module_param(month, int, 0);
MODULE_PARM_DESC(month, "target month");
module_param(date, int, 0);
MODULE_PARM_DESC(date, "target date");

EXPORT_SYMBOL(ts);

int init_module(void) {
	int mdays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

	if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)
		mdays[1] = 29;                       /* 閏年 2 月 29 天 */

	/* 先確認 month 在 1~12，才能用 mdays[month - 1]（|| 短路求值） */
	if (year < 1970 || month < 1 || month > 12 ||
	    date < 1 || date > mdays[month - 1]) {
		printk(KERN_ERR "Invalid argument(s).\n");
		return -EINVAL;                      /* 回傳負數 → insmod 失敗 */
	}

	ts = mktime64(year, month, date, 0, 0, 0);
	printk(KERN_INFO "deadline insert successfully! %d/%d/%d ts=%lld\n",
	       year, month, date, ts);
	return 0;
}

void cleanup_module(void) {
	printk(KERN_INFO "deadline remove successfully!\n");
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Lab5 exam: deadline");
```
`countdown.c`
```c
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
```
`Makefile`（指令前是 **TAB**）
```make
obj-m += deadline.o
obj-m += countdown.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```
**操作順序**
```bash
make
modinfo deadline.ko                                   # 看得到 parm: year:target year (int)…
sudo insmod deadline.ko year=2026 month=2 date=29     # 錯誤示範
sudo insmod deadline.ko year=2026 month=12 date=25
sudo insmod countdown.ko                              # 一定在 deadline 之後
sudo dmesg | tail -4
lsmod | head -3                                       # deadline 的 Used by = 1 countdown
sudo rmmod countdown                                  # 先拆用的人
sudo rmmod deadline
make clean
```

**容易踩的坑**
1. 減法方向寫反：`now - ts` 會變負的，結果全部走到「已經過了」那條。
2. `diff < 0` 時直接拿負數去 `/`、`%`：C 的除法向 0 取整，`%` 會出現負數，所以先 `diff = -diff` 再拆。
3. 小時數看起來「差 8 小時」：`mktime64` 把 12/25 00:00 當 UTC（＝台灣 08:00），這是正常的。若助教要求以台灣午夜為準，在 deadline 裡寫 `ts = mktime64(...) - 8 * 3600;`（純算術，不需要新 API）。
4. 兩個 module 檔名、`obj-m`、`insmod`／`rmmod` 的名字要一致；`rmmod` 不加 `.ko`。
