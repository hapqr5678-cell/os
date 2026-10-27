# Lab7_ex2:Exercise 2(50 分):Shared Memory producer / consumer

> 課堂 Exercise 原題(Lab7-2.pdf p.26–28)

## 題目

- `producer.c`:建立 shared memory;使用者輸入訊息寫進 shared memory。
- `consumer.c`:把 shared memory 的內容印在螢幕上。
- 編譯要加 `-lrt`。

## 編譯 / 執行

```bash
gcc -o producer producer.c -lrt
gcc -o consumer consumer.c -lrt
./producer
./consumer
```
