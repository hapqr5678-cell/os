# Lab1_g3

> 來源:review/predict/Lab1_predict.md

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
