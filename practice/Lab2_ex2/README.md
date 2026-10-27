# Lab2_ex2:Exercise-2(40 分):SYSCALL_DEFINE1 印 n 次 hello

> 課堂 Exercise 原題(簡報 p.42–43)

## 題目

改寫範例的 `hello.c`,使用 `SYSCALL_DEFINE1()`,讓 syscall 可接受一個參數,參數 = 輸出 hello 的次數。
例:`syscall(462, 3)` 即為輸出 3 次 hello。改好後要重新 `make -j $(nproc)`、`sudo make -j $(nproc) modules_install`、`sudo make install`、`reboot`。

## 編譯 / 執行

```bash
cd ~/linux && make -j $(nproc) && sudo make -j $(nproc) modules_install && sudo make install && reboot
gcc test.c -o test && ./test
sudo dmesg | tail
```
