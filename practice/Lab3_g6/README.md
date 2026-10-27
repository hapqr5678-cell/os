# Lab3_g6

> 來源:review/predict/Lab3_predict.md

## 第 6 題：用 C library（fopen/fread/fwrite/fclose）改寫檔案複製

### 題目
把第 3 題的 mycp 改用 `fopen/fread/fwrite/fclose` 實作，開檔失敗用 `perror` 印錯誤並 `exit(1)`。完成後回答：兩個版本用 `strace -c` 看，誰的 read/write 次數多？

**輸入／輸出範例**
```
$ ./q6 big.bin d6.bin
copied 10000 bytes
```

### 變形自哪裡／考什麼
- **example3（p.26~28）**：fwrite_file.c（`"w+"`、`perror`、`exit(1)`）＋ fread_file.c（`"rb"`、`fread(buffer, 1, sizeof(buffer), fp)`）。
- **p.22~25** 四個函式的參數與回傳值（fopen 失敗回 `NULL`；fread 回傳「元素個數」）。

### 可能性：低
沒有「只准用 unistd.h」那種限制的挑戰性，太像照抄 example3；比較可能當成第 3 題的追加問題。

### 參考答案（q6.c）
```c
// Q6：用 C library (fopen/fread/fwrite/fclose) 改寫檔案複製
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s <src> <dst>\n", argv[0]);
        return 1;
    }
    FILE *in = fopen(argv[1], "rb");
    if (in == NULL) { perror("fopen src"); exit(1); }
    FILE *out = fopen(argv[2], "wb");
    if (out == NULL) { perror("fopen dst"); fclose(in); exit(1); }

    char buffer[100];
    size_t n;
    long total = 0;
    while ((n = fread(buffer, 1, sizeof(buffer), in)) > 0) {  // 回傳「元素個數」
        fwrite(buffer, 1, n, out);
        total += n;
    }
    fclose(in);
    fclose(out);
    printf("copied %ld bytes\n", total);
    return 0;
}
```

### ✅ 已驗證
```
$ gcc -Wall -Wextra q6.c -o q6
$ ./q6 big.bin d6.bin && cmp big.bin d6.bin && echo SAME
copied 10000 bytes
SAME
```

### 容易踩的坑
1. `fread(buffer, sizeof(buffer), 1, in)`（size 和 count 對調）：回傳值變成 0 或 1，最後不滿一塊時會回 0 → 尾巴被吃掉。要用 `fread(buffer, 1, sizeof(buffer), in)`。
2. fopen 的 mode：`"w"` 會清空、`"r"` 不存在會失敗；二進位檔加 `b`（Linux 上沒差，但寫了比較標準）。
3. `fopen` 失敗回傳 **`NULL`**（不是 -1）；`open` 失敗才是 -1。
4. 本題 buffer 只開 100，但 strace 還是只看到少量 read，因為 stdio 自己的 buffer 是 4096。
