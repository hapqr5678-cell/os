# Lab4_ex:Exercise 1 static library + Exercise 2 shared library(sort)

> 課堂 Exercise 原題(簡報 p.14–15、p.26–27)

## 題目

**Exercise 1**
- `sort.c`:建立 sort 函式,參數接受陣列的位址,將陣列內的整數由小排到大。
- `exercise.c`(本資料夾為 `ex1.c`):建立長度 5 的陣列,放入 5 個不重複隨機整數,先印出 sort 前的值,再呼叫 sort 並印出結果。
- 將 `sort.o` 加入 `exlib.a`,用 gcc 把 exercise.c 與 exlib.a 輸出成 `exercise1`(本資料夾為 `ex1`)。

**Exercise 2**
- 用同樣兩支程式,改用 shared library 編譯(本資料夾為 `exlib.so`、`ex_dy`),成功後用 `ls -l` 比較檔案大小。

## 編譯 / 執行

```bash
# static
gcc -c sort.c
ar rcs exlib.a sort.o
gcc ex1.c -L. exlib.a -o ex1
# dynamic
gcc -c -fPIC sort.c
gcc sort.o -shared -o exlib.so
gcc ex1.c -L. exlib.so -o ex_dy
sudo cp exlib.so /lib            # 不然執行時找不到 .so
ldd ex_dy
ls -l ex1 ex_dy
```
