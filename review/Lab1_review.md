# Lab 1 複習：Shell Script（Vim + Bash）

> 來源：Lab1.pptx（54 張投影片）＋ practice/lab1/ex.sh

---

## 1. 這個 LAB 在學什麼

學會在 Linux terminal 用 **Vim** 編輯檔案，並把多個指令寫進一個文字檔變成 **shell script**，交給 shell（Bash）逐行解讀執行。重點語法：變數、`expr` 算術、三種引號、`read`、命令列參數、`if`／`test`、比較運算子、`for`／`while`、`case`、function、pipe `|`。Exercise 是用 shell script 寫「猜數字遊戲」。

---

## 2. 核心觀念

### 2.1 Shell 與 Shell Script
- **Shell**：使用者和 OS 之間的「指令解譯器」。你打一行指令，shell 解讀後請 OS 執行。Windows 對應的是 CMD。
- **Shell script**：把很多指令寫在一個文字檔裡，讓 shell 一次照順序執行。最常見用途是**把要重複做的事自動化**。
- **Bash**：GNU 專案寫的自由軟體 Unix shell，是大多數 Linux 發行版（與舊版 macOS）的預設 shell。
- 第一行 `#!/bin/sh` 或 `#!/bin/bash` 叫 **shebang**，告訴系統要用哪個直譯器跑這個檔案。（補充：Ubuntu 的 `/bin/sh` 其實是 `dash`，不支援 `((i++))`、`for ((...))` 這類 bash 專屬語法；用到這些時 shebang 要寫 `#!/bin/bash`。）

### 2.2 Vim 的三種模式（必考）
| 模式 | 如何進入 | 用途 |
|---|---|---|
| Normal mode（一般模式） | 開檔預設；在其他模式按 `Esc` | 移動、下指令前的狀態 |
| Insert mode（編輯模式） | Normal 下按 `i` | 輸入文字，左下角會顯示 `-- INSERT --` |
| Command-line mode（命令模式） | Normal 下按 `:` | `:w` 存檔、`:q` 離開、`:wq` 存檔並離開 |

（補充：`:q!` 不存檔強制離開。）

### 2.3 執行 script 需要「執行權限」
寫好的 `.sh` 預設沒有 x 權限，要先 `chmod u+x helloworld.sh`（或 `chmod +x`），才能用 `./helloworld.sh` 執行。`./` 代表「目前目錄下的這個檔案」。

### 2.4 變數
- **System variables**：系統內建，如 `$PWD`（目前目錄）、`$HOME`、`$PATH`。
- **User defined variables (UDV)**：自己定義，例如 `no=10`，用 `echo $no` 取值。
- **賦值時等號兩邊不能有空格**：`no=10` 對；`no = 10` 錯（shell 會把 `no` 當指令）。
- **比較時等號兩邊一定要有空格**：`[ "$a" = "$b" ]`。

### 2.5 `expr` 算術
- 語法：`expr op1 運算子 op2`，**運算子兩邊要有空格**：`expr 1 + 3` → `4`。
- 乘法要寫 `\*`：因為 `*` 在 shell 會被展開成「目前目錄的所有檔名」（wildcard），加 `\` 讓它被當一般字元。
- 要把運算結果存進變數要用反引號：`i=\`expr $i + 1\``。

### 2.6 三種引號（必考）
| 引號 | 行為 | 例子（`var=hi`） | 輸出 |
|---|---|---|---|
| 雙引號 `"` | 會展開變數 `$` 和反引號 `` ` ``，其他特殊字元當純文字 | `echo "$var, there"` | `hi, there` |
| 單引號 `'` | **全部**當純文字 | `echo '$var, there'` | `$var, there` |
| 反引號 `` ` `` | 執行裡面的指令，把結果放回原處 | ``echo "`expr 1 + 3`"`` | `4` |

其他投影片示範：`echo "expr 1 + 3"` → `expr 1 + 3`（只是字串）；``echo '`expr 1 + 3`'`` → `` `expr 1 + 3` ``（單引號連反引號也不展開）。

### 2.7 命令列參數
| 符號 | 意義 |
|---|---|
| `$0` | script 本身的名稱 |
| `$1`, `$2`… | 第 1、2… 個參數 |
| `$#` | 參數個數 |

### 2.8 `test` / `[ expr ]`
- `test 運算式` 與 `[ 運算式 ]` 等價，用來判斷運算式真假。
- **回傳 0 代表 true，非 0 代表 false**（和 C 語言相反，必考）。
- `[` 後面、`]` 前面都要有空格。

### 2.9 Pipe `|`
把 A 的輸出直接接到 B 的輸入，不需要暫存檔。例：`ls | sort -r` 反向排序列出目錄內容。

---

## 3. 指令／語法速查表

| 名稱 | 用途 | 重要參數／說明 | 範例 |
|---|---|---|---|
| `vim 檔名` | 開啟／建立檔案 | `i` 編輯、`Esc` 回 normal、`:w` `:q` `:wq` | `vim helloworld.sh` |
| `sudo apt-get update` | 更新套件列表 | — | `sudo apt-get update` |
| `sudo apt-get install` | 安裝套件 | — | `sudo apt-get install vim` |
| `echo` | 輸出字串 | 引號規則見上 | `echo Hello World!` |
| `chmod` | 改權限 | `u+x` 給擁有者執行權 | `chmod u+x a.sh` |
| `read` | 從鍵盤讀入變數 | 可一次讀多個：`read v1 v2` | `read fname lname` |
| `expr` | 整數運算 | `+ - \* /`，運算子兩側要空格 | ``x=`expr $a \* 2` `` |
| `test` / `[ ]` | 條件判斷 | 0=true | `[ "$a" -lt "$b" ]` |
| `-eq -ne -lt -le -gt -ge` | 數值比較 | 等於／不等於／小於／小於等於／大於／大於等於 | `[ $i -lt 6 ]` |
| `=`, `!=` | 字串比較 | 兩邊要空格 | `[ "$T1" = "$T2" ]` |
| `-n str` / `-z str` | 字串長度 >0 / =0 | — | `[ -z "$x" ]` |
| `if … then … elif … else … fi` | 條件 | `fi` 結尾 | 見下 |
| `for … in …; do … done` | 迴圈 | 也可 `for ((i=1;i<6;i++))`（bash） | `for i in 1 2 3; do echo $i; done` |
| `while [ ]; do … done` | 迴圈 | 遞增可用 ``i=`expr $i + 1` `` 或 `((i++))`、`((i=i+1))`、`((i+=1))` | 見下 |
| `case $var in pat) cmd;; esac` | 多分支 | `*)` 為預設；每個分支 `;;` 結尾 | 見下 |
| 函式 | `Name() { … }` 呼叫時只寫 `Name` | — | `PrintNumber` |
| `A \| B` | Pipe | A 的 stdout → B 的 stdin | `ls \| sort -r` |
| 附錄常用 | `ls cd pwd mkdir touch cat cp mv rm find grep` | `cd ~` 回家目錄 | — |
| 快捷鍵 | `Ctrl+Alt+T` 開 terminal、`Ctrl+C` 強制停止、`Ctrl+L` 清畫面、`Ctrl+D` 關閉、`Ctrl+S` 鎖定/`Ctrl+Q` 解鎖 | — | — |

---

## 4. 程式範例逐行解說

### 4.1 Hello World script
```bash
#!/bin/bash
echo "Hello World!"
```
步驟：`vim helloworld.sh` → `i` 輸入 → `Esc` → `:wq` → `chmod u+x helloworld.sh` → `./helloworld.sh` → 印出 `Hello World!`。

### 4.2 參數（投影片 myshell.sh）
```bash
echo "Welcome to use $0"
echo "Hi, $1 $2, we have $# argument(s)."
```
執行 `./myshell.sh nelson yeh` →
```
Welcome to use ./myshell.sh
Hi, nelson yeh, we have 2 argument(s).
```
`$0` 是你**打的路徑**（含 `./`），`$#` 不算 `$0`。

### 4.3 if / elif / else
```bash
if [ "foo" = "bar" ]; then
   echo foo eq bar
elif [ "foo" = "foo" ]; then
   echo foo eq foo
else
   echo foo not eq foo
fi
```
輸出：`foo eq foo`。第一個條件假、第二個真。

### 4.4 while 迴圈
```bash
i=1
while [ $i -lt 6 ]; do
     echo Item $i
     i=`expr $i + 1`
done
```
輸出 `Item 1` ~ `Item 5`（5 行）。條件 `-lt 6` 表示 i<6。

### 4.5 case（name.sh）
```bash
case $1 in
  "Amy")    echo "Amy Lin";;
  "Jimmy")  echo "Jimmy Wang";;
  "Johnny") echo "Johnny Chen";;
  "Purple") echo "Purple Red";;
  *)        echo "Error"
esac
```
`./name.sh Amy` → `Amy Lin`；`./name.sh Bob` → `Error`。最後一個分支的 `;;` 可省略。

### 4.6 函式
```bash
num=10
PrintNumber() {
    echo "The number is $num!"
}
PrintNumber
```
輸出 `The number is 10!`。函式要**先定義再呼叫**；shell 變數預設是全域的，所以函式內看得到 `num`。

### 4.7 Pipe 範例（ex3.sh | ex4.sh）
```bash
#!/bin/sh
#ex3.sh
echo "Amy"; echo "Jimmy"; echo "Johnny"; echo "Purple"
```
```bash
#!/bin/sh
#ex4.sh
while read first ; do
    case $first in
        "Amy")echo "Amy Lin";;
        "Jimmy")echo "Jimmy Wang";;
        "Johnny")echo "Jhonny Chen";;
        "Purple")echo "Purple Red";;
        *)echo "error"
    esac
done
```
`./ex3.sh | ./ex4.sh` → ex3 的 4 行輸出變成 ex4 的輸入，`while read` 一次讀一行，直到 EOF 結束。輸出 `Amy Lin / Jimmy Wang / Jhonny Chen / Purple Red`（投影片原文把 Johnny 拼成 Jhonny，是投影片的 typo，不影響邏輯）。

### 4.8 學生作業 practice/lab1/ex.sh（猜數字）
```bash
#!/bin/sh
echo "Please enter the target number between 1 and 10"
read target
min=1
max=10
while [ $min -le $max ]; do
	echo "Please guess a number between $min and $max"
	read guess
	if [ $guess -gt $target ]; then
		max="`expr $guess - 1`"
		echo "Too high!"
	elif [ $guess -lt $target ]; then
		min="`expr $guess + 1`"
		echo "Too low!"
	else
		echo "Bingo!"
		max="`expr $min - 1`"
	fi
done
```
逐行：
- 讀入目標 `target`，範圍初值 1~10。
- `while [ $min -le $max ]`：只要範圍還沒空就繼續猜。
- 猜太大 → 上界縮成 `guess-1`；太小 → 下界變 `guess+1`。
- 猜中 → 把 `max` 設成 `min-1` 讓迴圈條件變假而離開（取代 `break` 的技巧）。

範例互動（target=5）：猜 8 → `Too high!`，提示變 `between 1 and 7`；猜 3 → `Too low!`，提示 `between 4 and 7`；猜 5 → `Bingo!`。

**需要注意的問題：**
1. **沒有使用 function**：Exercise (2/3) 明確要求「Use function to ask for guessing number」，這份程式沒有定義任何函式，demo 時可能被扣分。建議改成下面的參考解。
2. **沒檢查輸入**：猜的數字超出 `[min,max]` 也照算（例如目前範圍 4~7 卻輸入 9，會把 max 改成 8，範圍反而變大）；輸入非數字時 `[ ... -gt ... ]` 會報錯 `integer expression expected`。
3. 沒檢查 target 是否在 1~10。
4. 用 `#!/bin/sh` 沒問題，因為只用了 POSIX 語法（`expr`、`[ ]`）。

### 4.9 Exercise 參考解法（含 function 與輸入檢查）
```bash
#!/bin/bash
# NGG.sh - Number Guessing Game
min=1
max=10

ask_guess() {            # 用 function 詢問猜測值
    echo "Please guess a number between $min and $max"
    read guess
}

echo "Please enter the target number between 1 and 10:"
read target
while [ "$target" -lt 1 ] || [ "$target" -gt 10 ]; do
    echo "Out of range, enter again:"
    read target
done

while true; do
    ask_guess
    if [ "$guess" -lt "$min" ] || [ "$guess" -gt "$max" ]; then
        echo "Out of range!"
        continue
    fi
    if [ "$guess" -gt "$target" ]; then
        echo "Too high!"
        max=`expr $guess - 1`
    elif [ "$guess" -lt "$target" ]; then
        echo "Too low!"
        min=`expr $guess + 1`
    else
        echo "Bingo!"
        break
    fi
done
```

---

## 5. 上機考實作步驟

```bash
# 0. 開 terminal：Ctrl + Alt + T
# 1. (需要時) 安裝 vim
sudo apt-get update
sudo apt-get install vim

# 2. 建立 script
vim NGG.sh          # 按 i 輸入程式 → Esc → :wq

# 3. 給執行權限並執行
chmod u+x NGG.sh
./NGG.sh

# 4. (或不改權限直接用 bash 跑)
bash NGG.sh

# 5. 測 pipe
./ex3.sh | ./ex4.sh
ls | sort -r
```

除錯技巧（補充）：`bash -x NGG.sh` 會把每一行執行前的內容印出來，方便看變數值。

---

## 6. 常見錯誤與陷阱

| 錯誤寫法 | 問題 | 正確 |
|---|---|---|
| `no = 10` | 賦值有空格 → `no: command not found` | `no=10` |
| `[ "$a"="$b" ]` | 比較沒空格 → 永遠為真（被當成一個非空字串） | `[ "$a" = "$b" ]` |
| `[$a -lt 5]` | `[` 後、`]` 前沒空格 | `[ $a -lt 5 ]` |
| `expr 2*3` | 沒空格且 `*` 沒跳脫 | `expr 2 \* 3` |
| `i=expr $i + 1` | 少了反引號 | ``i=`expr $i + 1` `` |
| `echo '$var'` 以為會印值 | 單引號不展開 | `echo "$var"` |
| `./a.sh: Permission denied` | 沒執行權限 | `chmod u+x a.sh` |
| `#!/bin/sh` 裡寫 `((i++))` | dash 不支援（補充） | 改用 `#!/bin/bash` |
| `if [ ... ] then` | `then` 前少 `;` 或換行 | `if [ ... ]; then` |
| case 分支忘了 `;;` | 語法錯誤 | 每個分支結尾 `;;` |
| 用 `>` 比數字：`[ $a > $b ]` | `>` 在 `[ ]` 裡是**重新導向**，會建立檔案 | `-gt` |
| 變數沒加引號、值為空 | `[ = foo ]` 語法錯 | `"$var"` |

---

## 7. 筆試模擬題

**Q1（選擇）** Vim 中，從 insert mode 回到 normal mode 要按？
(A) `:`　(B) `i`　(C) `Esc`　(D) `Ctrl+C`
> **答：C**。`i` 是 normal → insert；`:` 是 normal → command-line。

**Q2（選擇）** 想「存檔並離開」Vim，應在命令模式輸入？
(A) `:q`　(B) `:w`　(C) `:wq`　(D) `:x!`
> **答：C**。`:w` 只存、`:q` 只離開。

**Q3（是非）** 在 shell 中 `test` 指令回傳 0 代表 false。
> **答：錯**。回傳 0 代表 **true**，非 0 為 false。

**Q4（輸出預測）**
```bash
var=hi
echo "$var, there"
echo '$var, there'
echo "`expr 1 + 3`"
echo '`expr 1 + 3`'
```
> **答：**
> ```
> hi, there
> $var, there
> 4
> `expr 1 + 3`
> ```
> 雙引號展開 `$` 與反引號；單引號全部原樣。

**Q5（簡答）** 為什麼 `expr 3 * 4` 可能出錯？如何修正？
> **答：** `*` 會被 shell 展開成目前目錄所有檔名（wildcard）。改寫 `expr 3 \* 4`，用 `\` 跳脫。

**Q6（輸出預測）** 執行 `./myshell.sh a b c`：
```bash
echo "$0 $# $2"
```
> **答：** `./myshell.sh 3 b`。

**Q7（選擇）** 下列哪一個是「數值小於等於」比較運算子？
(A) `-lt`　(B) `-le`　(C) `-ge`　(D) `<=`
> **答：B**。`-lt` 小於、`-ge` 大於等於；`<=` 不能在 `[ ]` 中比數字。

**Q8（是非）** `count = 5` 可以把 5 存進變數 count。
> **答：錯**。賦值時等號兩邊不能有空格，應寫 `count=5`。

**Q9（輸出預測）**
```bash
i=1
while [ $i -lt 4 ]; do
  echo Item $i
  i=`expr $i + 1`
done
echo $i
```
> **答：** `Item 1`、`Item 2`、`Item 3`，最後一行 `4`。

**Q10（簡答）** 解釋 `ls | sort -r` 的作用與 pipe 的優點。
> **答：** 把 `ls` 的輸出當作 `sort -r` 的輸入，反向排序列出檔名。Pipe 不需要中間的暫存檔就能串接程式。

**Q11（選擇）** `[ -z "$str" ]` 何時為真？
(A) str 非空　(B) str 長度為 0　(C) str 等於 "z"　(D) str 為數字
> **答：B**。`-n` 是長度 > 0。

**Q12（簡答）** 寫好 `a.sh` 後執行 `./a.sh` 出現 `Permission denied`，原因與解法？
> **答：** 檔案沒有執行權限。`chmod u+x a.sh`（或用 `bash a.sh` 直接交給 bash 執行）。
