# Lab6_ex1:Exercise 1(40 分):fork 後 global / local 變數

> 課堂 Exercise 原題(簡報 p.22–24)

## 題目

- 宣告一個 global 變數和一個 local 變數,初值都是 10。
- 在螢幕上印出兩個變數的值(附 process id 與 parent 的 id)。
- 建立一個 child process,印出兩個變數,然後把兩個變數遞增。
- 最後兩個 process 都印出 global 與 local 的值,附自己的 PID 與 PPID。
- (對照 p.24 Result:只有 child 遞增,parent 印出的仍是 10。)

## 編譯 / 執行

```bash
gcc ex1.c -o ex1 && ./ex1
```
