# Lab7_ex1:Exercise-Lab1(50 分):Message Queue sender / receiver

> 課堂 Exercise 原題(Lab7-1.pdf p.22–24)

## 題目

- `sender.c`:建立 message queue;使用者可反覆輸入字串,直到輸入 `"exit"`;把訊息送到 message queue。
- `receiver.c`:反覆從 message queue 接收訊息並顯示在螢幕上;最後 close & unlink message queue。
- 可能用到:`fgets(buf, BUFSIZ, stdin)`、`strcmp(message, "hello")`。

## 編譯 / 執行

```bash
gcc sender.c -o sender -lrt
gcc receiver.c -o receiver -lrt
./receiver        # 終端機 B
./sender          # 終端機 A
```
