# Lab 5 上機考猜題：Kernel Module

> 依據：`Lab5.pptx`（36 頁，含投影片內的程式截圖與講者備註）＋ `practice/lab5/`（d2ts.c、myage.c、Makefile）＋ `review/Lab5_review.md`
> 前提：只出簡報教過的東西。本檔用到的 kernel API 只有：`printk`（KERN_INFO／KERN_ERR／KERN_WARNING）、`module_param`、`MODULE_PARM_DESC`、`EXPORT_SYMBOL`／`extern`、`MODULE_LICENSE`、`return -EINVAL`、`mktime64`（p.35 提示），再加上 exercise 要你「取得當前時間戳記」（p.32）一定要用、但簡報沒寫名字的 `ktime_get_real_seconds()`（練習答案就是用它）。
>
> **⚠️ 無法在此環境編譯驗證**：這台機器沒有 `/lib/modules`，也沒有 kernel headers，所有 `.c` 都**沒有**經過真正的 kbuild／insmod 測試。我做了下面兩件事來補：
> 1. **整份 .c 原封不動**用假的 `<linux/*.h>`（自己寫的 mock，`mktime64` 逐字照抄 kernel 原始碼）在 user space 編譯（`gcc -Wall -Wextra`，零 warning），再用自製的迷你 insmod／rmmod 模擬器跑過每一題的指令順序（含相依檢查、參數八進位解析、`-EINVAL`）。
> 2. 把純計算邏輯抽出來，跟 glibc 的 `timegm`／`gmtime` 做窮舉比對（結果見 §4）。
>
> 測試檔都在 `/tmp/claude-0/-home-user-os/1dcd3112-7322-5920-934d-1535c60cff9f/scratchpad/lab5/`（`run_all.sh` 一鍵重跑，輸出在 `results.txt`）。

---

## 1. Lab 5 專屬的出題邏輯（補充共通邏輯 1~4）

| # | 觀察 | 從哪裡看出來 | 對猜題的意義 |
|---|---|---|---|
| A | **Exercise ＝ hello_module_2 ＋ symbol_A/B 直接合體**。date2timestamp 的參數就是 hello_module_2 的 `year/month/date`（連名字都一樣）；`EXPORT_SYMBOL(ts)` ＋ myage 的 `extern` ＋ 安裝順序就是 symbol_A/B。 | p.19、p.25~29、p.31~32 | 變形題的骨架幾乎確定是「**A 模組：參數＋驗證＋export；B 模組：extern＋計算＋印出**」，只換「參數是什麼、export 什麼、B 算什麼」。 |
| B | **symbol_A 示範了兩種 symbol（variable＋function），exercise 只用了 variable**。 | p.23「variable symbol / function symbol」、p.25 `symbol_function`、p.31 只 export `ts` | 「教了但 exercise 沒用到」的那一半（**export 函式**）是最自然的變形點。你的練習答案自己加了 `is_adult()`，方向完全正確。 |
| C | **Exercise 比範例多出的兩個新東西**：`return -EINVAL` 中止安裝、`mktime64`。 | p.35 提示 | 驗收一定會 demo「**錯誤輸入被擋下來**」（p.33 範例輸出第一步就是 `year=1970 month=13 date=-1`），驗證規則可能換（時分秒範圍、兩個日期都要驗…）。 |
| D | **kernel 裡沒有浮點數、沒有 libc**，能做的計算只有整數 `/`、`%`。時間 API 只給了 `mktime64` ＋取得現在時間。 | p.32「timestamp 的精度為秒」、p.35 | 計算題只會是「**秒數 ↔ 年/日/時/分/秒**」「兩個時間戳相減」「倒數」「星期幾」這類整數運算，不會出要浮點或字串處理的題目。 |
| E | **驗收看 dmesg**：「其餘文字不須照打，看得懂即可」。 | p.33 | 數字對、錯誤情境有擋下、`dmesg | tail -N` 叫得出來就過關；不要花時間對齊範例文字。 |
| F | **安裝／移除順序本身就是觀念題**（Used by、`is in use by`）。 | p.28~29 | 助教可能叫你「故意先 rmmod A 給我看會怎樣」或「lsmod 指出誰被誰用」。 |
| G | **modinfo 會顯示 MODULE_PARM_DESC**；講者備註提到 `08` 被當八進位的 bug。 | p.20、p.21 備註 | 可能叫你 `modinfo` 秀出參數說明；demo 時參數**不要打前導 0**。 |
| H | 範例輸出的 `ts=63163929600`（2001/8/1）**不是** Unix 時間（`mktime64` 算出來是 `996624000`，比對過 glibc）。 | p.33 截圖 | 自己的數字跟範例不一樣不用慌，以 `mktime64` 為準；被問就說「我用的是 Unix epoch 秒數」。 |

**第 0 題（不列入排序，但一定要會）：原題重做。** 把 `practice/lab5/` 的 d2ts.c 改名成 `date2timestamp.c`（Makefile 的 `obj-m` 一起改），10 分鐘內要能從零默寫出來。參考答案見 `review/Lab5_review.md` §5。以下 6 題都是它的變形。

---

## 2. 猜題（依可能性排序）

> 所有範例輸出都以「**現在 = 2026-10-28 10:00:00（台灣時間）= 1793152800**」計算（來自模擬器實跑）。實際考試時數字會隨當下時間改變。
> **時區提醒**：`ktime_get_real_seconds()` 是 UTC 秒數，`mktime64()` 也把輸入當 UTC。所以「2026/12/25 00:00」其實是台灣 08:00。只要兩邊都是 UTC，**天數**就不會錯；只有要精確到「小時」時才會看到 8 小時的差（見各題的坑）。

### 總表

| 題號 | 題目 | 變形自 | 可能性 |
|---|---|---|---|
| Q1 | 倒數計時：距離某日期還有多久（deadline ＋ countdown） | exercise（相減方向反過來） | **高** |
| Q2 | 加上時分秒參數，算出活了幾年幾天幾時幾分幾秒 | exercise（換參數＋換計算） | **高** |
| Q3 | export **函式**：datelib 提供 `is_valid_date`／`date2ts`，daydiff 帶兩組日期參數算相差天數 | symbol_A/B 的 function symbol ＋ exercise | **中高** |
| Q4 | 精確足歲＋下次生日倒數（export 參數變數本身） | exercise ＋ 你自己加的 is_adult | **中** |
| Q5 | 你出生那天是星期幾、活了幾週 | exercise（換 B 的計算） | **中低** |
| Q6 | 單一 module：輸入日期，印出是該年第幾天、是否閏年 | hello_module_2 ＋ exercise 驗證 | **低** |

---

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

---

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

---

### Q3. Export 函式：兩個日期相差幾天（可能性：中高）

**題目敘述**
- `datelib`：不需要參數。export 兩個**函式**：`int is_valid_date(int y, int m, int d)`（有效回傳 1，否則 0）、`time64_t date2ts(int y, int m, int d)`（回傳時間戳記）。
- `daydiff`：參數 `year1 month1 date1 year2 month2 date2`，呼叫 datelib 的函式驗證兩個日期（任一無效 → 錯誤訊息＋終止安裝），印出兩個日期相差幾天（日期 2 早於日期 1 時為負數）。

```bash
sudo insmod daydiff.ko                      # 先裝 daydiff → 失敗
sudo insmod datelib.ko
sudo insmod daydiff.ko year1=2001 month1=8 date1=1 year2=2026 month2=10 date2=28
sudo dmesg | tail -3
sudo rmmod datelib                          # 故意先拆 datelib → 失敗
```
預期輸出：
```
insmod: ERROR: could not insert module daydiff.ko: Unknown symbol in module
[...] datelib insert successfully!
[...] daydiff insert successfully!
[...] From 2001/8/1 to 2026/10/28: 9219 days
rmmod: ERROR: Module datelib is in use by: daydiff
```
其他測資：`2024/1/1 → 2025/1/1：366 days`（閏年）、`2026/12/25 → 2026/10/28：-58 days`、`year1=2025 month1=2 date1=29` → `Invalid argument(s).`

**變形自／考哪個觀念**
- p.23 `void doSomething()` → function symbol、p.25 `symbol_function` 被 export、p.26 B 端 `extern int symbol_function(int);`。exercise 只 export 變數，這題補上函式那一半（出題邏輯 B）。
- 參數放在 **B（使用者）端**，而且 B 也要能 `return -EINVAL`——觀念：任何一個 module 的 `init_module` 回傳負值都會讓它裝不起來（p.9 註解「A non 0 return means init_module failed」）。
- 安裝／移除順序（p.28~29）。

**可能性：中高**。理由：symbol_A 的範例本身就是 export 函式，exercise 沒用到，很適合拿來「驗證一個觀念」（共通邏輯 2）。你自己練習時加 `is_adult()` 也證明這是自然的延伸。扣分點：要寫兩個帶參數的函式原型，比 Q1／Q2 多一點東西。

**參考答案** ⚠️ 無法在此環境編譯驗證（已用 mock 在 user space 編譯＋模擬執行通過）

`datelib.c`
```c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/time64.h>

/* function symbol 1：日期有效回傳 1，無效回傳 0 */
int is_valid_date(int y, int m, int d) {
	int mdays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

	if ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0)
		mdays[1] = 29;
	if (y < 1970 || m < 1 || m > 12 || d < 1 || d > mdays[m - 1])
		return 0;
	return 1;
}

/* function symbol 2：日期 → 時間戳記（秒） */
time64_t date2ts(int y, int m, int d) {
	return mktime64(y, m, d, 0, 0, 0);
}

EXPORT_SYMBOL(is_valid_date);
EXPORT_SYMBOL(date2ts);

int init_module(void) {
	printk(KERN_INFO "datelib insert successfully!\n");
	return 0;
}

void cleanup_module(void) {
	printk(KERN_INFO "datelib remove successfully!\n");
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Lab5 exam: datelib");
```
`daydiff.c`
```c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/time64.h>

extern int is_valid_date(int y, int m, int d);   /* 來自 datelib */
extern time64_t date2ts(int y, int m, int d);

int year1 = 2001, month1 = 8, date1 = 1;
int year2 = 2026, month2 = 10, date2 = 28;

module_param(year1, int, 0);
MODULE_PARM_DESC(year1, "year of date 1");
module_param(month1, int, 0);
MODULE_PARM_DESC(month1, "month of date 1");
module_param(date1, int, 0);
MODULE_PARM_DESC(date1, "date of date 1");
module_param(year2, int, 0);
MODULE_PARM_DESC(year2, "year of date 2");
module_param(month2, int, 0);
MODULE_PARM_DESC(month2, "month of date 2");
module_param(date2, int, 0);
MODULE_PARM_DESC(date2, "date of date 2");

int init_module(void) {
	time64_t diff;

	if (!is_valid_date(year1, month1, date1) ||
	    !is_valid_date(year2, month2, date2)) {
		printk(KERN_ERR "Invalid argument(s).\n");
		return -EINVAL;
	}

	diff = date2ts(year2, month2, date2) - date2ts(year1, month1, date1);
	printk(KERN_INFO "daydiff insert successfully!\n");
	printk(KERN_INFO "From %d/%d/%d to %d/%d/%d: %lld days\n",
	       year1, month1, date1, year2, month2, date2, diff / 86400);
	return 0;
}

void cleanup_module(void) {
	printk(KERN_INFO "daydiff remove successfully!\n");
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Lab5 exam: daydiff");
```
`Makefile`
```make
obj-m += datelib.o
obj-m += daydiff.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```
**操作順序**
```bash
make
modinfo daydiff.ko                 # 6 個 parm；depends: datelib
sudo insmod datelib.ko
sudo insmod daydiff.ko year1=2001 month1=8 date1=1 year2=2026 month2=10 date2=28
sudo dmesg | tail -3
lsmod | head -3                    # datelib  ...  1 daydiff
sudo rmmod daydiff
sudo rmmod datelib
```

**容易踩的坑**
1. `extern` 的函式原型要和定義一致（回傳型別 `time64_t`、參數個數）。寫錯不一定編譯失敗，但結果會是垃圾值。
2. B 端用到 `time64_t`，記得 `#include <linux/time64.h>`（B 自己不呼叫 mktime64 也要型別）。
3. 參數名稱不要取 `y1`：gcc 有同名的內建數學函式 `y1()`，可能出 warning；用 `year1` 最安全。
4. 被 export 的函式／變數**不要加 `static`**（static 表示只在本檔可見，和「要給別的 module 用」互相矛盾；照簡報 symbol_A 的寫法不加就對了）。
5. 新版 kernel（6.8 起）對沒有原型的全域函式會出 `no previous prototype` **warning**，不影響編譯（簡報的 `symbol_function` 一樣會有）。
6. 輸出「天數」時兩個時間戳都是午夜，相減一定是 86400 的倍數，負數直接 `/ 86400` 沒問題。

---

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

---

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

---

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

---

## 3. 跨題共通的坑（考場上最常卡住的）

| 症狀 | 原因 | 解法 |
|---|---|---|
| `Makefile:4: *** missing separator` | 指令前是空白不是 TAB（p.10） | vim 裡 `:set list` 看，改成 TAB |
| `insmod: ERROR: … Unknown symbol in module` | ① 相依的 A 還沒裝 ② A 忘了 `EXPORT_SYMBOL` ③ 參數名打錯（dmesg 會寫 `Unknown parameter`） | `sudo dmesg \| tail` 看是哪一種 |
| `insmod: ERROR: … Invalid parameters` | ① 自己 `return -EINVAL`（正確行為） ② 參數有前導 0（`08`、`09`） | 看 dmesg 是 `Invalid argument(s).` 還是 `` `08' invalid for parameter `` |
| `insmod: ERROR: … File exists` | 同名 module 已經載入 | 先 `rmmod` |
| `rmmod: ERROR: Module X is in use by: Y` | 還有人在用 X（p.29） | 先 `rmmod Y` |
| `Operation not permitted` | 忘了 `sudo` | — |
| dmesg 看不到東西 | `printk` 字串沒有 `\n`；或 `KERN_INFO` 後面加了逗號 | `printk(KERN_INFO "…\n", …)`，中間**沒有逗號** |
| `disagrees about version of symbol module_layout` | 版本警告，簡報截圖也有（p.21、p.28） | 可忽略，module 仍會載入 |
| 編譯 error：`time64_t` unknown | 沒 include | `#include <linux/time64.h>` |
| `ktime_get_real_seconds` implicit declaration | 沒 include | `#include <linux/timekeeping.h>` |
| `printk` 印 `ts` 出 warning | 格式字元錯 | `time64_t` 用 `%lld`，`int` 用 `%d` |
| 想用 `printf`、`<stdio.h>`、`float` | kernel 沒有 libc、不用浮點 | 只用 `printk` 與整數 `/`、`%` |

另外：所有答案都把 `year < 1970` 視為無效（和你的練習答案一致，也和 p.33 範例用 `year=1970 month=13 …` 測錯誤一致）。這是**自訂規則**，題目沒要求；`mktime64` 其實算得出 1970 以前的負數時間戳。被問到要能說明：「1970 以前時間戳是負的，後面除法、星期幾會有負數取整的問題，所以我直接擋掉。」

---

## 4. 驗證狀況

**⚠️ 所有 kernel module 都無法在此環境編譯驗證**（`/lib/modules` 不存在，沒有 kernel headers），以下是替代驗證：

**(1) 整份 .c 在 user space 編譯＋模擬執行**
- 用假的 `<linux/module.h>` 等標頭（`kmock.h`）：`printk` → `printf`、`module_param` → 登錄到參數表、`EXPORT_SYMBOL` → 空、`mktime64` → **逐字照抄 `kernel/time/time.c`**、`ktime_get_real_seconds()` → 固定回傳 1793152800（2026-10-28 10:00 台灣）。
- 6 題共 11 個 .c，`gcc -std=gnu11 -Wall -Wextra` 編譯 **0 error、0 warning**；另用 `-Wdeclaration-after-statement -Wshadow -Wstrict-prototypes` 檢查也沒有問題（`-Wmissing-prototypes` 只對 export 的函式報 warning，見 Q3 坑 5）。
- 自製迷你 insmod／rmmod（`kmock.c`）模擬：每次 insmod 參數回到預設值、`kstrtoint` 的 base 0 解析（`08` 會失敗）、未知參數、相依模組未載入 → `Unknown symbol`、被使用時 rmmod → `in use by`、重複 insmod → `File exists`、init 回傳 `-EINVAL` → `Invalid parameters`。本檔所有「預期輸出」都是從這個模擬器實跑複製來的。
- 先拿你的 `practice/lab5/d2ts.c`、`myage.c` 原檔跑一次確認模擬器正確：`year=2001 month=8 date=1` → `ts=996624000`、`25 years and 94 days`、`You are an adult (18+).`；`year=1970 month=13 date=-1` → `Invalid argument(s).`；`year=2023 month=2 date=29` → 無效；`year=2024 month=2 date=29` → 有效。

**(2) 純計算邏輯 vs glibc 窮舉比對（`logic_test.c`）**
```
[1] 日期有效性判斷：195048 組，錯 0 組
[2] mktime64 對照 timegm：109938 天，錯 0 天
    mktime64(2001,8,1)=996624000（投影片範例的 63163929600 不是 Unix 時間）
    mktime64(2025,2,29)==mktime64(2025,3,1)? 是（不會檢查日期，會自動進位）
    mktime64(2001,13,1)=1009843200 == mktime64(2002,1,1)=1009843200? → 月份 13 會被進位
[3] 秒數拆成 年/日/時/分/秒 再組回：4800 組，錯 0 組
[4] 星期幾公式：84371 天，錯 0 天
[5] 一年中第幾天：84371 天，錯 0 天
[6] 足歲迴圈：2597658 組，錯 0 組
[7] 2001/8/1 → 2026/10/28：總天數 9219；365 近似 = 25 年 94 天；真正 = 25 歲又 88 天（差 6 個閏日）
[8] (-1)/86400 = 0、(-1)%7 = -1 → 1970 以前要另外處理（答案用 year<1970 擋掉）
```

**(3) 沒辦法驗證的部分**（明天上機時第一次 `make` 就會知道）
- kbuild／modpost 是否有額外 warning（例如 6.8+ 的 `-Wmissing-prototypes`；只是 warning）。
- 真實 kernel 對 export 名稱 `year`／`month`／`date`（Q4）是否和其他已載入模組衝突（正常情況不會）。
- 64 位元除法：考場 VM 是 x86_64（簡報截圖 `6.5.0-41-generic`），`time64_t` 直接 `/`、`%` 沒問題；若是 32 位元 ARM 才需要 `div_s64`（不在範圍內）。

---

## 5. 考前 5 分鐘速查

**Makefile 模板**（兩個 module；只有一個就刪掉一行 `obj-m`；**指令前是 TAB**）
```make
obj-m += A.o
obj-m += B.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```

**A 模組骨架（參數＋驗證＋export）**
```c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/time64.h>

int year = 2000, month = 1, date = 1;
time64_t ts = 0;
module_param(year, int, 0);
MODULE_PARM_DESC(year, "birth year");
/* month、date 同上 */
EXPORT_SYMBOL(ts);

int init_module(void) {
	int mdays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)
		mdays[1] = 29;
	if (year < 1970 || month < 1 || month > 12 || date < 1 || date > mdays[month - 1]) {
		printk(KERN_ERR "Invalid argument(s).\n");
		return -EINVAL;
	}
	ts = mktime64(year, month, date, 0, 0, 0);
	printk(KERN_INFO "A insert successfully! ts=%lld\n", ts);
	return 0;
}
void cleanup_module(void) { printk(KERN_INFO "A remove successfully!\n"); }
MODULE_LICENSE("GPL");
```
**B 模組骨架（extern＋現在時間）**
```c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/time64.h>
#include <linux/timekeeping.h>
extern time64_t ts;
int init_module(void) {
	time64_t diff = ktime_get_real_seconds() - ts;
	if (diff < 0) { printk(KERN_WARNING "not born yet!\n"); return 0; }
	printk(KERN_INFO "%lld years %lld days\n", diff / 86400 / 365, diff / 86400 % 365);
	return 0;
}
void cleanup_module(void) { printk(KERN_INFO "B remove successfully!\n"); }
MODULE_LICENSE("GPL");
```

**指令順序**
```bash
make                                    # 產生 A.ko、B.ko
modinfo A.ko                            # 看 parm（MODULE_PARM_DESC）
sudo insmod A.ko year=2001 month=8 date=1   # 參數不要打 08；insmod 要加 .ko
sudo insmod B.ko                        # 被依賴的先裝
lsmod | head -3                         # A 的 Used by：1 B
sudo dmesg | tail -5                    # 看輸出（tail -N 看最後 N 行）
sudo rmmod B                            # 依賴者先拆；rmmod 不加 .ko
sudo rmmod A
make clean
```

**module_param 型別**（簡報只示範 `int`，其餘為補充，考試用 `int` 就夠）

| type | C 變數型別 | insmod 寫法 |
|---|---|---|
| `int` / `uint` | `int` / `unsigned int` | `year=2001` |
| `long` / `ulong` | `long` / `unsigned long` | `n=123456789` |
| `short` / `ushort` | `short` / `unsigned short` | `n=12` |
| `bool` | `bool` | `flag=1`、`flag=Y` |
| `charp` | `char *` | `name=hello` |

**常用數字**：1 分 = 60 秒、1 時 = 3600 秒、1 天 = 86400 秒、1 週 = 604800 秒、1970/1/1 是星期四、2001/8/1 → `ts=996624000`。

**printk 格式**：`int` → `%d`、`time64_t`／`long long` → `%lld`；`KERN_INFO` 與字串之間**沒有逗號**；字串結尾記得 `\n`。
