# Lab1_g2

> 來源:review/predict/Lab1_predict.md

## Q2【加料版猜數字】範圍與次數改用參數給，並統計猜了幾次

**可能性：高**。符合「同一個骨架再加一個簡報觀念」：把 p.32 的 `$1 $2 $#` 跟 `$0` 塞進 exercise，再多一個計數器。

### 題目
寫 `guess.sh`，用法是 `./guess.sh min max limit`：
1. 如果參數不是 3 個，印出 `Usage: ./guess.sh min max limit`（要用 `$0`）然後結束。
2. 輸入目標數字後，最多讓玩家猜 `limit` 次，每次提示 `[第幾次/limit]` 跟目前的範圍（用 function）。
3. 猜中時印 `Bingo! You got it in N tries.`。次數用完時印 `Game over! The answer is X.`。

```
$ ./guess.sh 1 100 5
Please enter the target number between 1 and 100:
42
[1/5] Please enter a number between 1 and 100
50
Too high!
[2/5] Please enter a number between 1 and 49
25
Too low
[3/5] Please enter a number between 26 and 49
42
Bingo! You got it in 3 tries.
```

**變形自：** Exercise（p.48–49），加上 p.32 參數。**考的觀念：** `$0 $1 $2 $3 $#`、`-ne`、`while [ $count -lt $limit ]`（p.40 的計數迴圈）、``count=`expr $count + 1` ``、字串 `=`（p.38）。

### 參考答案
```bash
#!/bin/bash
# Q2: 猜數字進階版 —— 範圍由參數給、限制次數、統計次數
if [ $# -ne 3 ]; then
    echo "Usage: $0 min max limit"
    exit 1
fi

min=$1
max=$2
limit=$3
count=0
win=no

ask_guess() {
    echo "[$count/$limit] Please enter a number between $min and $max"
    read guess
}

echo "Please enter the target number between $min and $max:"
read target

while [ $count -lt $limit ]; do
    count=`expr $count + 1`
    ask_guess
    if [ "$guess" -gt "$target" ]; then
        echo "Too high!"
        max=`expr $guess - 1`
    elif [ "$guess" -lt "$target" ]; then
        echo "Too low"
        min=`expr $guess + 1`
    else
        win=yes
        break
    fi
done

if [ "$win" = "yes" ]; then
    echo "Bingo! You got it in $count tries."
else
    echo "Game over! The answer is $target."
fi
```

### ✅ 已驗證（bash 跟 sh/dash 輸出相同）
```
$ printf '42\n50\n25\n42\n' | ./q2_ngg2.sh 1 100 5
Please enter the target number between 1 and 100:
[1/5] Please enter a number between 1 and 100
Too high!
[2/5] Please enter a number between 1 and 49
Too low
[3/5] Please enter a number between 26 and 49
Bingo! You got it in 3 tries.

$ printf '42\n50\n25\n' | ./q2_ngg2.sh 1 100 2
...
[2/2] Please enter a number between 1 and 49
Too low
Game over! The answer is 42.

$ ./q2_ngg2.sh 1 2
Usage: ./q2_ngg2.sh min max limit
```

### 容易踩的坑
- `$#` 是**參數的個數**，不包含 `$0`。
- `count` 要先設成 `0`。變數沒有初始化的話，`[ $count -lt 5 ]` 會出現 `unary operator expected` 的錯誤。
- `exit` 簡報沒教，但它是結束 script 最直接的寫法。不想用的話，可以把整支程式包進 `if [ $# -ne 3 ]; then ... else ... fi`。
- 數字比較要用 `-lt`/`-gt`，不能用 `<`/`>`。在 `[ ]` 裡面，`>` 會被當成輸出導向。實測 `[ 5 > 3 ]` 會產生一個叫 `3` 的檔案，而且判斷結果永遠是真。
