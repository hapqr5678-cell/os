# Lab2_ex1:Exercise-1(60 分):完成範例,新增 hello syscall

> 課堂 Exercise 原題(簡報 p.41)

## 題目

照簡報步驟新增 `sys_hello`(`SYSCALL_DEFINE0`,印出 `Hello`),編譯安裝 kernel,用 `syscall(462)` 測試並找助教 demo。

要改的檔案:
- `~/linux/hello/hello.c`、`~/linux/hello/Makefile`(`obj-y += hello.o`)
- `~/linux/include/linux/syscalls.h`:加 `asmlinkage long sys_hello(void);`
- `~/linux/Kbuild`:加 `obj-y += hello/`
- `~/linux/arch/x86/entry/syscalls/syscall_64.tbl`:加 `462 common hello sys_hello`

## 編譯 / 執行

```bash
cd ~/linux && make -j $(nproc) && sudo make -j $(nproc) modules_install && sudo make install && reboot
gcc test.c -o test && ./test      # 印出 0
sudo dmesg | tail                 # 看到 Hello
```
