# Lab3_ex:只用 <unistd.h> 印出 integer

> 課堂 Exercise 原題(簡報 p.32)

## 題目

- 學習目標:了解為何我們比較偏好使用將 system call 包裝後的函式。
- 要求:在只使用 `<unistd.h>` 的情況下印出 integer(助教會檢查有沒有偷用 `<stdio.h>`)。
- 測資:`0`、`100`、`2147483647`、`-2147483648`。

## 編譯 / 執行

```bash
gcc ex.c -o ex && ./ex
```
