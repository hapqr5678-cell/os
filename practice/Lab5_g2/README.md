# Lab5_g2

> 來源:review/predict/Lab5_predict.md

### Q2. 加上時分秒：活了幾年幾天幾時幾分幾秒（可能性：高）

**題目敘述**
- `date2timestamp`：參數改成 `year month date hour minute second` 六個（hour 0~23、minute／second 0~59），超出範圍視為無效 → 錯誤訊息＋終止安裝；export `ts`。
- `myage`：輸出你活了「X 年 Y 天 Z 時 W 分 S 秒」（一年以 365 天計）；若 ts 在未來要提示。

```bash
sudo insmod date2timestamp.ko year=2001 month=8 date=1 hour=24             # hour 超出範圍
sudo insmod date2timestamp.ko year=2001 month=8 date=1 hour=13 minute=30 second=5
sudo insmod myage.ko
sudo dmesg | tail -4
```
預期輸出：
```
insmod: ERROR: could not insert module date2timestamp.ko: Invalid parameters
[...] Invalid argument(s).
[...] date2timestamp insert successfully! ts=996672605
[...] myage insert successfully!
[...] You have lived 25 years 93 days 12 hours 29 minutes 55 seconds!
```
ts 在未來（例如 `year=2026 month=10 date=28 hour=3`）：`You are not born yet! (diff=-3600)`

**變形自／考哪個觀念**
- 共通邏輯 1「換參數與計算內容」：參數從 3 個變 6 個（p.17~19 的 `module_param` 多寫幾次）、驗證規則變多（p.35）、`mktime64` 的後三個參數終於不是 `0, 0, 0`（p.35 提示裡就有這三個位置）。
- p.32「timestamp 的精度為秒」→ 秒數拆成 年/日/時/分/秒 的整數運算。

**可能性：高**。理由：`mktime64(year, month, date, 0, 0, 0)` 的三個 0 本身就是很明顯的「下次換這裡」；改動只在參數與輸出格式，最像「原題換皮」。

**參考答案** ⚠️ 無法在此環境編譯驗證（已用 mock 在 user space 編譯＋模擬執行通過）

`date2timestamp.c`
```c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/time64.h>

int year = 2000, month = 1, date = 1;
int hour = 0, minute = 0, second = 0;        /* 新增的三個參數 */
time64_t ts = 0;

module_param(year, int, 0);
MODULE_PARM_DESC(year, "birth year");
module_param(month, int, 0);
MODULE_PARM_DESC(month, "birth month");
module_param(date, int, 0);
MODULE_PARM_DESC(date, "birth date");
module_param(hour, int, 0);
MODULE_PARM_DESC(hour, "birth hour (0-23)");
module_param(minute, int, 0);
MODULE_PARM_DESC(minute, "birth minute (0-59)");
module_param(second, int, 0);
MODULE_PARM_DESC(second, "birth second (0-59)");

EXPORT_SYMBOL(ts);

int init_module(void) {
	int mdays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

	if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)
		mdays[1] = 29;

	if (year < 1970 || month < 1 || month > 12 ||
	    date < 1 || date > mdays[month - 1] ||
	    hour < 0 || hour > 23 || minute < 0 || minute > 59 ||
	    second < 0 || second > 59) {
		printk(KERN_ERR "Invalid argument(s).\n");
		return -EINVAL;
	}

	ts = mktime64(year, month, date, hour, minute, second);
	printk(KERN_INFO "date2timestamp insert successfully! ts=%lld\n", ts);
	return 0;
}

void cleanup_module(void) {
	printk(KERN_INFO "date2timestamp remove successfully!\n");
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Lab5 exam: date2timestamp with time");
```
`myage.c`
```c
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
```
`Makefile`
```make
obj-m += date2timestamp.o
obj-m += myage.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```
**操作順序**
```bash
make
modinfo date2timestamp.ko          # 6 個 parm
sudo insmod date2timestamp.ko year=2001 month=8 date=1 hour=24
sudo insmod date2timestamp.ko year=2001 month=8 date=1 hour=13 minute=30 second=5
sudo insmod myage.ko
sudo dmesg | tail -4
sudo rmmod myage
sudo rmmod date2timestamp
```

**容易踩的坑**
1. 參數名 `minute`、`second` 一定要和 insmod 打的完全一樣；打錯（例如 `min=30`）kernel 會回 `Unknown parameter`，insmod 失敗。
2. `hour=08`、`minute=09` → 前導 0 被當八進位，`08`/`09` 不合法 → `Invalid parameters`（p.21 備註）。**demo 時一律不打前導 0。**
3. 拆解順序：先 `% 60` 拿秒，再 `/= 60`，一路往上；最後才 `/ 365`、`% 365`。順序錯數字會亂。
4. 時區：輸入的 13:30 會被當 UTC（＝台灣 21:30）。要當成台灣時間就 `ts = mktime64(...) - 8 * 3600;`。助教若用手機時間對答案，「時」可能差 8，要能解釋。
5. 驗證條件很長，`||` 之間不要漏；`date > mdays[month - 1]` 一定要放在 `month` 檢查之後（短路求值，避免陣列越界）。
