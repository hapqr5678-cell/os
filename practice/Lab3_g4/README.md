# Lab3_g4

> 來源:review/predict/Lab3_predict.md

## 第 4 題：用 open() 指定權限建立檔案，把整數寫進檔案

### 題目
只用 `<unistd.h>`、`<fcntl.h>`：建立 `score.txt`，權限必須是 **`rw-r-----`（owner 讀寫、group 只讀、other 無）**，把 1~5 的平方一行一個寫進檔案。完成後螢幕印 `done`，並用 `ls -l`、`cat` 給助教看。

**輸入／輸出範例**
```
$ ./q4
done
$ ls -l score.txt
-rw-r----- 1 user user 12 ... score.txt
$ cat score.txt
1
4
9
16
25
```
（延伸口試題：`chmod 0244 score.txt` 之後 `cat` 會怎樣？→ `Permission denied`，因為 owner 只剩 w，p.12 示範過。）

### 變形自哪裡／考什麼
- **p.10~12 權限碼**：`rw-r-----` → 4+2 / 4 / 0 → **0640**；要會「權限字串 ↔ 八進位」互轉。
- **p.13** open 的第三個參數 mode。
- **Exercise** 的 my_print 改成「可以指定 fd」：印到螢幕是 fd 1，印到檔案是 open 回傳的 fd → 體會「一切都是檔案」（p.3）。

### 可能性：中
權限碼在簡報占了 3 頁，但單獨考 `chmod` 太簡單，最可能是和 open 的 mode 結合。

### 參考答案（q4.c）
```c
// Q4：用 open() 指定權限 0640 建立 score.txt，把 1~5 的平方寫進檔案（一行一個）
// 只能用 <unistd.h> 與 <fcntl.h>
#include <unistd.h>
#include <fcntl.h>

void my_fprint_int(int fd, int a) {      // 印到「任意 fd」：1=螢幕，或 open 回傳的檔案
    char buf[12];
    int i = 11;
    unsigned int u = (a < 0) ? -(unsigned int)a : (unsigned int)a;
    do { buf[i--] = '0' + u % 10; u /= 10; } while (u > 0);
    if (a < 0) buf[i--] = '-';
    write(fd, buf + i + 1, 11 - i);
}

int main() {
    int fd = open("score.txt", O_CREAT | O_WRONLY | O_TRUNC, 0640);
    if (fd == -1) {
        write(2, "open failed\n", 12);
        return 1;
    }
    for (int k = 1; k <= 5; k++) {
        my_fprint_int(fd, k * k);
        write(fd, "\n", 1);
    }
    close(fd);
    write(1, "done\n", 5);
    return 0;
}
```

### ✅ 已驗證
```
$ gcc -Wall -Wextra -Werror=implicit-function-declaration q4.c -o q4
$ ./q4
done
$ ls -l score.txt
-rw-r----- 1 root root 12 Oct 27 12:01 score.txt
$ cat score.txt
1
4
9
16
25
```

### 容易踩的坑
1. **umask**：open 實際權限 = `mode & ~umask`。預設 umask 是 `022`，所以 `0666` 會變 `0644`、`0777` 會變 `0755`。題目要 `0640` 沒受影響；但若題目要 `0666`，你會看到 `rw-r--r--`——口頭說明「被 umask 022 遮掉」即可。
2. **檔案已存在時 mode 不會改**：open 的 mode 只在「新建」時生效。重測前先 `rm score.txt`，否則看到的是舊權限。
3. `0640` 不能寫成 `640`（十進位 640 = 八進位 1200，完全不同）。
4. 數字寫進檔案要轉成字元，`write(fd, &x, sizeof(int))` 會寫進 4 個二進位 bytes，`cat` 出來是亂碼。
