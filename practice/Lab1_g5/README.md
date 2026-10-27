# Lab1_g5

> 來源:review/predict/Lab1_predict.md

## Q5【命令列參數 + expr】三個數字：加總、最大值、階乘

**可能性：中**。p.32 的 `myshell.sh`（`$0 $1 $2 $#`）是有「執行結果」的範例。加上 p.28 的 `\*`，再用 function 包一個 while 迴圈，就是很典型的小題。

### 題目
寫 `nums.sh`，用法 `./nums.sh n1 n2 n3`：
1. 第一行印 `Welcome to use $0`。參數不是 3 個時印 `Usage: ...` 並結束。
2. 印出參數個數、三個數的和、最大值。
3. 用 function `fact` 搭配 while 迴圈，計算 `$1` 的階乘。

```
$ ./nums.sh 5 3 9
Welcome to use ./nums.sh
Hi, we have 3 argument(s): 5 3 9
sum = 17
max = 9
5! = 120
```

**變形自：** p.32 myshell.sh，加上 p.40 while 計數迴圈（`i=`expr $i + 1``）。**考的觀念：** `$0 $1 $2 $3 $#`、`expr` 連加、`\*`、`-gt`、function 裡的 `$1` 是**函式自己的參數**。

### 參考答案
```bash
#!/bin/bash
# Q5: 命令列參數 $0 $1 $2 $3 $# —— 加總、最大值、階乘
echo "Welcome to use $0"
if [ $# -ne 3 ]; then
    echo "Usage: $0 n1 n2 n3"
    exit 1
fi
echo "Hi, we have $# argument(s): $1 $2 $3"

sum=`expr $1 + $2 + $3`
echo "sum = $sum"

max=$1
if [ $2 -gt $max ]; then
    max=$2
fi
if [ $3 -gt $max ]; then
    max=$3
fi
echo "max = $max"

# 階乘:function + while + expr \*
fact() {
    f=1
    i=1
    while [ $i -le $1 ]; do
        f=`expr $f \* $i`
        i=`expr $i + 1`
    done
    echo "$1! = $f"
}
fact $1
```

### ✅ 已驗證（bash 跟 sh/dash 輸出相同）
```
$ ./q5_args.sh 5 3 9
Welcome to use ./q5_args.sh
Hi, we have 3 argument(s): 5 3 9
sum = 17
max = 9
5! = 120

$ ./q5_args.sh 1 2
Welcome to use ./q5_args.sh
Usage: ./q5_args.sh n1 n2 n3
```

### 容易踩的坑
- `$0` 印出的是**你執行時打的路徑**（`./nums.sh`），不一定只有檔名（p.32）。
- **function 裡的 `$1` 是呼叫 function 時傳進去的參數**，跟 script 的 `$1` 不同。這裡用 `fact $1` 把 script 的 `$1` 傳進去，所以剛好是同一個值。
- `expr $1 + $2 + $3` 可以連加。每個運算元和運算子之間都要有空格。
- 階乘一定要用 `\*`。這是 p.28 唯一特別強調的陷阱。
