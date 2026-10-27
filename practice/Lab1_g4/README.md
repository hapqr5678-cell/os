# Lab1_g4

> 來源:review/predict/Lab1_predict.md

## Q4【Pipe】兩支 script 串接：產生分數 | 判定等第並統計

**可能性：中**。p.46 是整份簡報**最後一個範例**，而且正好是兩支 script 串起來（`./ex3.sh | ./ex4.sh`）。exercise 完全沒用到 pipe，最適合出成「做出 p.46 的變形」。

### 題目
1. `gen.sh`：用 for 迴圈依序輸出 `95 82 60 45 73`，每行一個（仿 ex3.sh）。
2. `grade.sh`：用 `while read` 從標準輸入一行一行讀分數（仿 ex4.sh），印出 `分數 -> 等第`（>=90 A、>=80 B、>=60 C、其他 F）。讀完後印出筆數、及格人數、平均（整數）。
3. 執行 `./gen.sh | ./grade.sh`。

```
$ ./gen.sh | ./grade.sh
95 -> A
82 -> B
60 -> C
45 -> F
73 -> C
count=5 pass=4 avg=71
```

**變形自：** p.45–46 `ex3.sh | ex4.sh`（把名字換成分數，把 case 對照換成 if/elif 區間判斷）。**考的觀念：** pipe（p.45）、`while read`（p.46）、`for i in ...`（p.39）、`-ge`（p.37）、elif（p.36）、expr 累加跟 `/`（p.28）。

### 參考答案
```bash
#!/bin/bash
# Q4 producer (仿 ex3.sh):每行輸出一個分數
for s in 95 82 60 45 73; do
    echo $s
done
```
```bash
#!/bin/bash
# Q4 consumer: 從 stdin 讀分數,判定等第,最後印統計
total=0
n=0
pass=0
while read s; do
    if [ "$s" -ge 90 ]; then
        g=A
    elif [ "$s" -ge 80 ]; then
        g=B
    elif [ "$s" -ge 60 ]; then
        g=C
    else
        g=F
    fi
    echo "$s -> $g"
    total=`expr $total + $s`
    n=`expr $n + 1`
    if [ $s -ge 60 ]; then
        pass=`expr $pass + 1`
    fi
done
if [ $n -eq 0 ]; then
    echo "No input"
else
    echo "count=$n pass=$pass avg=`expr $total / $n`"
fi
```

### ✅ 已驗證（bash 跟 sh/dash 輸出相同，`./` 直接執行也一樣）
```
$ ./q4_gen.sh | ./q4_grade.sh
95 -> A
82 -> B
60 -> C
45 -> F
73 -> C
count=5 pass=4 avg=71

$ printf '' | ./q4_grade.sh
No input
```
另外驗證了 p.46 原範例的寫法（`while read first; do case $first in "Amy") ...`）：輸入 `Amy`、`Bob`，輸出 `Amy Lin`、`error`。

### 容易踩的坑
- **兩支 script 都要 `chmod u+x`**，不然 `./gen.sh | ./grade.sh` 會出現 `Permission denied`（p.25）。
- `while read s` 讀到 EOF 時會自動結束，不需要另外判斷。
- 判斷等第時，**if/elif 的順序要從大到小**，不然 95 會先符合 `-ge 60`。
- `$n` 是 0 時，`expr $total / $n` 會出現除以零的錯誤，要先判斷。
- 小提醒：bash 裡 pipe 右邊是 subshell，迴圈內改的變數**在 pipe 外面看不到**。這題的統計寫在 `grade.sh` 裡面，整支都在 pipe 右邊，所以沒有問題。
