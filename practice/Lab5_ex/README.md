# Lab5_ex:date2timestamp + myage

> 課堂 Exercise 原題(簡報 p.31–35)

## 題目

**date2timestamp**(本資料夾為 `d2ts.c`)
- module parameters:`year`、`month`、`date`;export symbol:`ts`。
- 輸入出生年月日,印出該日期的時間戳記 `ts`,並提供給其他模組使用。
- 若輸入無效日期,顯示錯誤訊息並終止安裝(`printk(KERN_ERR "Invalid argument(s).\n"); return -EINVAL;`)。

**myage**
- 取得當前時間戳記,與 `ts` 相減,轉成年跟日,輸出你已經活多久;相減小於 0 要提示。
- 提示:`#include <linux/time64.h>`,`mktime64(year, month, date, 0, 0, 0);` 時間戳精度為秒。

(本資料夾的答案多了一個簡報沒要求的 `is_adult()`,是自己練習的延伸。)

## 編譯 / 執行

```bash
make
sudo insmod d2ts.ko year=2001 month=8 date=1
sudo insmod myage.ko
sudo dmesg | tail
sudo rmmod myage && sudo rmmod d2ts     # 先拆 myage 再拆 d2ts
```
