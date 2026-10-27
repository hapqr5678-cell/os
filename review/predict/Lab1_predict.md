# Lab 1（Shell Script）上機考猜題

> 範圍：`Lab1.pptx` 第 1–53 頁。參考答案：`practice/lab1/ex.sh`。
> 所有參考答案都在 scratchpad 實際跑過，`bash` 跑一次、`sh`（Ubuntu 的 `/bin/sh` 是 **dash**）再跑一次，並用 `printf '...' | ./xxx.sh` 模擬鍵盤輸入。
> 頁碼 = 投影片右下角的編號（例如 p.37 = 「Bash Shell Scripting Tutorial (19/28)」）。

---

## 0. Lab 1 專屬的出題邏輯（補充／修正）

### 0.1 Exercise 實際考了什麼（p.48–50）

| Exercise 要求 | 對應投影片 | 在程式裡的樣子 |
|---|---|---|
| echo 印提示 | p.20 | `echo "Please enter ..."` |
| read 讀輸入 | p.31 | `read target`、`read guess` |
| loop | p.39–40 | `while true; do ... done` 一直猜到中 |
| conditionals | p.33–37 | `if / elif / else` 加上 `-gt`、`-lt` |
| function 問猜測值 | p.44 | `ask_guess() { echo ...; read guess; }` |
| 範圍一直縮小 | p.28 + p.29 | ``max=`expr $guess - 1` ``、``min=`expr $guess + 1` `` |

結果截圖（p.50）的輸出格式：
```
Please enter the target number between 1 and 10:
5
Please guess a number between 1 and 10
8
Too high!
Please enter a number between 1 and 7
2
Too low
Please enter a number between 3 and 7
...
Bingo!
```
**觀察：** 這題的重點是「全域變數 `min`/`max` 被迴圈更新，function 每次印出來的範圍跟著變」。所以「function 直接讀全域變數」這個觀念（p.44 `PrintNumber` 印 `$num`）其實是 exercise 偷偷考的地方。

> 修正：`00_總覽與考前速查.md` 寫 `practice/lab1/ex.sh`「沒有用 function」，這是舊資訊。目前的 `ex.sh` **已經有** `in_range()`、`ask_guess()` 兩個 function，也有檢查輸入。不過它用了 `case ''|*[!0-9]*`、`!`、`&&`、`return` 這些簡報沒教的寫法。考場上照簡報的寫法就夠了。

### 0.2 簡報教了，但 exercise 沒考到（最可能拿來變形）

| 沒考到的觀念 | 頁碼 | 可能怎麼考 |
|---|---|---|
| **`case ... esac`**（有 3 頁範例） | p.41–43 | 選單、名字對照、成績等第 |
| **Pipe `\|` + `while read` 接 stdin** | p.45–46（ex3.sh \| ex4.sh） | 一支 script 產生資料，另一支讀進來處理 |
| **命令列參數 `$0 $1 $2 $#`** | p.32 | 範圍或次數改用參數給，並檢查 `$#` |
| **`expr` 的 `\*` 跟 `/`** | p.28 | 計算機、階乘、平均 |
| **字串比較 `= != -n -z`** | p.33–35、p.38 | 帳密檢查、空字串檢查 |
| **`read a b` 一次讀兩個變數** | p.31 | 一行輸入兩個數字 |
| `for i in 1 2 3`、`for ((i=1;i<6;i++))` | p.39 | 固定次數的迴圈、產生資料 |
| 三種引號 | p.29–30 | 輸出裡同時有 `$var` 原文和它的值（比較適合筆試） |
| vim / chmod / `./` | p.7–17、p.25–26 | 一定會用到，但不會單獨出成一題 |

### 0.3 Lab 1 的變形規則（推測）
1. **換外殼，留骨架**：骨架是「read → loop → if/case → 更新變數 → 用 function 包互動」。外殼可以換成猜數字、計算機、成績、登入。
2. **加一個 exercise 沒用到的語法**：最可能補的是 `case`（3 頁範例都沒用到），其次是 `$1/$#` 跟 pipe。
3. **原題直接重考或小幅加料**（限制次數、統計次數、範圍改用參數給）。這最省助教的事，機率最高。

---

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

---

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

---

## Q3【case + function + expr】選單式計算機

**可能性：中高**。`case` 有 3 頁投影片（p.41–43），exercise 卻完全沒用到。`expr` 的 `\*` 是簡報特別標紅的重點（p.28）。這題把兩個「沒考到」的東西合在一起，形式又很像 exercise（一直問直到某個條件成立）。

### 題目
寫 `calc.sh`：
1. 用 function `show_menu` 印出選單：`a) add  s) sub  m) mul  d) div  q) quit`。
2. 讀入選項，用 **case** 分支。a/s/m/d 要再讀「兩個數字」（同一行，用 `read x y`），然後呼叫 function `calc` 印出結果。
3. 除數是 0 時印 `Error: divide by zero`。輸入不認識的選項時印 `Invalid option`。輸入 q 時印 `Bye!` 並結束。
4. 一定要用 `expr` 計算。

```
$ ./calc.sh
===== Calculator =====
a) add  s) sub  m) mul  d) div  q) quit
Choose:
m
Enter two numbers:
4 6
4 * 6 = 24
...
Choose:
q
Bye!
```

**變形自：** 簡報 p.28 的 `expr 1 + 3 / 10 \* 3 / 12 / 4` 四則運算，加上 p.42 的 case 範例。**考的觀念：** p.41 case 語法（`;;`、`*)`）、p.28 `\*`、p.31 `read a b`、p.44 function、p.29 反引號。

### 參考答案
```bash
#!/bin/bash
# Q3: 選單式計算機 (read + case + function + expr + while)
show_menu() {
    echo "===== Calculator ====="
    echo "a) add  s) sub  m) mul  d) div  q) quit"
    echo "Choose:"
}

calc() {
    case $op in
        a) echo "$x + $y = `expr $x + $y`";;
        s) echo "$x - $y = `expr $x - $y`";;
        m) echo "$x * $y = `expr $x \* $y`";;
        d) if [ "$y" -eq 0 ]; then
               echo "Error: divide by zero"
           else
               echo "$x / $y = `expr $x / $y`"
           fi;;
    esac
}

while true; do
    show_menu
    read op
    case $op in
        a|s|m|d)
            echo "Enter two numbers:"
            read x y
            calc;;
        q)
            echo "Bye!"
            break;;
        *)
            echo "Invalid option";;
    esac
done
```

### ✅ 已驗證（bash 跟 sh/dash 輸出相同）
```
$ printf 'a\n3 5\nm\n4 6\nd\n7 0\nd\n17 5\ns\n2 9\nx\nq\n' | ./q3_calc.sh
（每輪都會先印選單，這裡省略）
3 + 5 = 8
4 * 6 = 24
Error: divide by zero
17 / 5 = 3
2 - 9 = -7
Invalid option
Bye!
```

### 容易踩的坑
- **`expr $x * $y` 沒有跳脫**：目錄裡有檔案時，`*` 會被展開成檔名。實測 `expr 3 * 5` 的結果是 `expr: syntax error: unexpected argument 'a.txt'`。一定要寫 `\*`。
- **`expr 3+5` 沒有空格**：會直接印出字串 `3+5`，不會計算（已實測）。運算子兩邊都要有空格。
- `expr` 只做**整數**運算，`17 / 5 = 3`。
- 每個 case 分支最後都要有 `;;`。`*)` 是預設分支，要放在最後。整段用 `esac` 結尾。
- 在雙引號裡面寫 `"$x * $y"` 時，`*` 不會被展開，可以直接印出來（p.29：雙引號只解讀 `$` 跟反引號）。
- `a|s|m|d)` 是用一個分支比對多個值（這是 case 的標準語法，簡報沒有特別示範，不想用的話可以分開寫成四個分支）。

---

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

---

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

---

## Q6【字串比較】帳號密碼檢查（最多 3 次）

**可能性：中低**。p.38 的字串運算子（`= != -n -z`）跟 p.33–35 的字串 if 範例，exercise 都沒用到（exercise 只有數字比較）。如果助教想考「字串跟數字比較的差別」，就會出這種題目。

### 題目
寫 `login.sh`：帳號 `admin`、密碼 `1234`。每次在同一行讀入「帳號 密碼」。
- 有一個是空的：印 `Username and password must not be empty`
- 帳號錯：印 `No such user: xxx`
- 密碼錯：印 `Wrong password`
- 成功：印 `Welcome, admin! (tries: N)` 並結束
- 失敗 3 次：印 `Too many attempts, locked.`

```
$ ./login.sh
Enter username and password:
root 1
No such user: root
Enter username and password:
admin 1234
Welcome, admin! (tries: 2)
```

**變形自：** p.35「Conditionals with variables」（`[ "$T1" = "$T2" ]`），加上 p.38 字串運算子、p.31 `read fname lname`。**考的觀念：** `=`、`!=`、`-z`，以及字串比較時**一定要用雙引號**。

### 參考答案
```bash
#!/bin/bash
# Q6: 帳密檢查 (字串比較 = != -z, read 兩個變數, while 計次)
USER_OK="admin"
PASS_OK="1234"
tries=0
ok=no
while [ $tries -lt 3 ]; do
    echo "Enter username and password:"
    read u p
    tries=`expr $tries + 1`
    if [ -z "$u" ] || [ -z "$p" ]; then
        echo "Username and password must not be empty"
    elif [ "$u" != "$USER_OK" ]; then
        echo "No such user: $u"
    elif [ "$p" = "$PASS_OK" ]; then
        ok=yes
        break
    else
        echo "Wrong password"
    fi
done
if [ "$ok" = "yes" ]; then
    echo "Welcome, $u! (tries: $tries)"
else
    echo "Too many attempts, locked."
fi
```

### ✅ 已驗證（bash 跟 sh/dash 輸出相同）
```
$ printf 'root 1\n\nadmin 1234\n' | ./q7_login.sh
Enter username and password:
No such user: root
Enter username and password:
Username and password must not be empty
Enter username and password:
Welcome, admin! (tries: 3)

$ printf 'admin 1\nadmin 2\nadmin 3\n' | ./q7_login.sh
Enter username and password:
Wrong password
Enter username and password:
Wrong password
Enter username and password:
Wrong password
Too many attempts, locked.
```

### 容易踩的坑
- **比較字串用 `=`，比較數字用 `-eq`**。用 `-eq` 比字串會出現 `integer expression expected` 的錯誤。
- **變數一定要加雙引號**：`x=""` 時，`[ $x = admin ]` 會變成 `[ = admin ]` 而出錯。`x="a b"` 時，`[ -n $x ]` 會出現 `binary operator expected` 的錯誤（已實測）。
- `[ "$a" = "$b" ]` 的 `=` **兩邊要有空格**。寫成 `"$a"="$b"` 會被當成一個非空字串，永遠為真（p.33）。
- 只按 Enter（空行）時 `u`、`p` 都是空字串，要用 `-z` 抓出來。

---

## 加碼：bash 限定語法（`for ((...))`、`((i++))`）

p.39–40 有教 `for ((i=1; i<6; i++))` 跟 `((i++))`。**但這些是 bash 的語法，`sh`（dash）不支援。**
```bash
#!/bin/bash
for ((i=1; i<=3; i++)); do
    line=""
    for ((j=1; j<=3; j++)); do
        line="$line$i*$j=`expr $i \* $j` "
    done
    echo "$line"
done
```
✅ 已驗證：
```
$ bash q6_table.sh
1*1=1 1*2=2 1*3=3
2*1=2 2*2=4 2*3=6
3*1=3 3*2=6 3*3=9
$ sh q6_table.sh
q6_table.sh: 3: Syntax error: Bad for loop variable
```
→ **用到 `(( ))` 時，第一行一定要寫 `#!/bin/bash`，並用 `./xxx.sh` 或 `bash xxx.sh` 執行，不要用 `sh xxx.sh`。**（p.46 的 ex3/ex4 是 `#!/bin/sh`，那兩支沒有用到 `(( ))`，所以沒問題。）

---

## 驗證總表

| 題目 | bash | sh (dash) | 備註 |
|---|---|---|---|
| Q1 原題猜數字 | ✅ | ✅ | 正常流程跟超出範圍都測過 |
| Q2 參數＋限制次數 | ✅ | ✅ | 猜中、次數用完、參數錯誤三種情況 |
| Q3 case 計算機 | ✅ | ✅ | 四則運算、除以 0、不認識的選項、離開 |
| Q4 pipe 成績 | ✅ | ✅ | `./gen \| ./grade` 跟空輸入 |
| Q5 參數 sum/max/階乘 | ✅ | ✅ | 正常情況跟參數個數錯誤 |
| Q6 帳密字串比較 | ✅ | ✅ | 成功跟鎖定 |
| 加碼 `for (( ))` | ✅ | ❌（預期） | 用來證明 `(( ))` 只能在 bash 跑 |

---

## 考前 5 分鐘速查

```bash
# ---- 開檔 / 執行 (p.7-17, p.22-26) ----
vim ngg.sh          # i 進 insert → Esc 回 normal → :wq 存檔離開（:q! 不存檔離開）
chmod u+x ngg.sh    # 沒有執行權限 → Permission denied
./ngg.sh 參數1 參數2  # 或 bash ngg.sh
#!/bin/bash         # 第一行；用到 (( )) 就必須是 bash

# ---- 變數 (p.27) ----
no=10               # 等號兩邊不能有空格
echo $no $PWD

# ---- 算術 (p.28) ----
a=`expr $a + 1`     # 運算子兩邊要有空格；只能算整數
expr 10 \* 3        # 乘號一定要寫成 \*
((i++))  ((i+=1))   # bash 限定

# ---- 引號 (p.29-30) ----
"$var `cmd`"        # 會展開變數跟反引號
'$var `cmd`'        # 全部照原樣印出
`cmd`               # 執行指令，把結果放回原位

# ---- 輸入 / 參數 (p.31-32) ----
read a b            # 一行讀兩個值
$0 程式名  $1 $2 參數  $# 參數個數

# ---- 條件 (p.33-38) ----
if [ "$a" -lt "$b" ]; then ... elif [ ... ]; then ... else ... fi
#   [ 跟 ] 內側要有空格；比較用的 = 兩邊要有空格
# 數字：-eq -ne -lt -le -gt -ge
# 字串：=  !=  -n(非空)  -z(空)
# 兩個條件：[ A ] || [ B ]      [ A ] && [ B ]

# ---- 迴圈 (p.39-40) ----
for i in 1 2 3; do ...; done
for ((i=1; i<6; i++)); do ...; done          # bash 限定
while [ $i -lt 6 ]; do i=`expr $i + 1`; done
while true; do ... break; done
while read line; do ...; done                 # 從 stdin / pipe 讀，讀到 EOF 結束

# ---- case (p.41-42) ----
case $x in
    "Amy") echo "Amy Lin";;
    a|b)   echo ab;;
    *)     echo error;;
esac

# ---- function (p.44) ----
ask() {             # 先定義才能呼叫；可以直接讀寫全域變數
    echo "range $min~$max"
    read guess
}
ask                 # 呼叫時不加括號；ask 5 → 函式裡用 $1 取得

# ---- pipe (p.45-46) ----
./ex3.sh | ./ex4.sh   # 左邊的輸出接到右邊的輸入
ls | sort -r
```

**看到錯誤訊息時馬上對照：**
| 錯誤訊息 | 原因 |
|---|---|
| `[5: command not found` | `[` 後面少了空格 |
| `a: command not found` | 賦值寫成 `a = 5`（等號旁邊有空格） |
| `unary operator expected` | 變數是空的，而且沒有加雙引號 |
| `expr: syntax error: unexpected argument 'xxx'` | `*` 沒有寫成 `\*` |
| 印出 `3+5` 而不是 8 | `expr 3+5` 沒有空格 |
| `Bad for loop variable` | 用 `sh` 跑了 `for ((...))` |
| `Permission denied` | 忘了 `chmod u+x` |
| `integer expression expected` | 用 `-eq`/`-lt` 比較了非數字的值 |
