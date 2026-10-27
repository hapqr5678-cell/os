# Lab1_g6

> 來源:review/predict/Lab1_predict.md

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
