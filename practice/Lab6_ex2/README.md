# Lab6_ex2:Exercise 2(50 分):5 個 process 的樹

> 課堂 Exercise 原題(簡報 p.25–27)

## 題目

- 寫一個程式建立 5 個 process,形成指定的樹狀結構(A → B、D、E;B → C)。
- 每個 process 印出自己的 process id 與 parent 的 process id。
- 有 child 的 process 必須依 child 的數目呼叫相同次數的 `wait()`,等所有 child 結束。

## 編譯 / 執行

```bash
gcc ex2.c -o ex2 && ./ex2
```
