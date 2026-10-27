# Lab5_g3

> 來源:review/predict/Lab5_predict.md

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
