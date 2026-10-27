# Lab1_g1

> 來源:review/predict/Lab1_predict.md

## Q1【原題重現】Number Guessing Game（1~10，範圍逐步縮小）

**可能性：高**。上機考常常就是 exercise 原題，或者只改提示文字。一定要能在 10 分鐘內默寫出來。

### 題目
寫 `NGG.sh`：
1. 先讓使用者輸入目標數字（1~10），超出範圍要重新輸入。
2. 用 **function** 詢問猜測值，提示目前的範圍。
3. 猜太大印 `Too high!`，範圍上限改成 guess-1。猜太小印 `Too low`，下限改成 guess+1。猜中印 `Bingo!` 並結束。
4. 必須用到 echo、read、loop、conditionals、function。

```
$ ./NGG.sh
Please enter the target number between 1 and 10:
5
Please enter a number between 1 and 10
8
Too high!
Please enter a number between 1 and 7
2
Too low
Please enter a number between 3 and 7
...
Bingo!
```

**變形自：** p.48–50 Exercise 本身。**考的觀念：** p.20 echo、p.31 read、p.40 while、p.36–37 if/elif + `-lt -gt`、p.44 function、p.28 expr、p.29 反引號。

### 參考答案
```bash
#!/bin/bash
# Q1: Number Guessing Game (原題標準版)
min=1
max=10

# 用 function 詢問猜測(題目明確要求)
ask_guess() {
    echo "Please enter a number between $min and $max"
    read guess
}

echo "Please enter the target number between 1 and 10:"
read target
while [ "$target" -lt 1 ] || [ "$target" -gt 10 ]; do
    echo "Out of range! Please enter the target number between 1 and 10:"
    read target
done

while true; do
    ask_guess
    if [ "$guess" -lt "$min" ] || [ "$guess" -gt "$max" ]; then
        echo "Out of range!"
    elif [ "$guess" -gt "$target" ]; then
        echo "Too high!"
        max=`expr $guess - 1`
    elif [ "$guess" -lt "$target" ]; then
        echo "Too low"
        min=`expr $guess + 1`
    else
        echo "Bingo!"
        break
    fi
done
```

### ✅ 已驗證（bash 跟 sh/dash 輸出相同）
```
$ printf '5\n8\n2\n6\n3\n5\n' | ./q1_ngg.sh
Please enter the target number between 1 and 10:
Please enter a number between 1 and 10
Too high!
Please enter a number between 1 and 7
Too low
Please enter a number between 3 and 7
Too high!
Please enter a number between 3 and 5
Too low
Please enter a number between 4 and 5
Bingo!

$ printf '15\n5\n11\n5\n' | ./q1_ngg.sh        # 超出範圍的情況
Please enter the target number between 1 and 10:
Out of range! Please enter the target number between 1 and 10:
Please enter a number between 1 and 10
Out of range!
Please enter a number between 1 and 10
Bingo!
```
（用管線餵輸入時，輸入的數字不會顯示在畫面上。實際用鍵盤打字時，會像 p.50 那樣看到數字。）

### 容易踩的坑
- **縮小範圍時寫成 `max=$guess`**：上下限要 ±1，不然範圍不會變小（p.50：猜 8 之後範圍是 1~7）。
- **`max=` 等號旁邊有空格**：`max = ...` 會被當成執行 `max` 這個指令（p.33：賦值的時候等號旁邊不能有空格）。
- **`[` 後面跟 `]` 前面要有空格**：寫成 `[$guess -gt $target]` 會出現 `[5: command not found`。
- function 要**寫在呼叫它的地方之前**（shell 是由上往下執行的）。
- `||` 連接的是兩個 `[ ]`，不能寫成 `[ a || b ]`。如果怕記錯，可以拆成兩個 `elif`。
- `break` 在簡報上沒有列出來，但跳出 `while true` 一定要用它。也可以改用旗標變數：`while [ "$guess" != "$target" ]`。
