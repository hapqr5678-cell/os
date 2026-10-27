# Lab4_g6

> 來源:review/predict/Lab4_predict.md

## 題 6：字串函式庫 reverse + to_upper

### 題目
撰寫 `reverse.c`（將字串反轉）與 `upper.c`（將小寫轉大寫），共用 `strlib.h`，打包成 `strlib.a` 與 `strlib.so`。`main.c` 讀入一行字串，依序印出原字串、反轉後、轉大寫後。分別編成 `str_static`、`str_dynamic`，`ls -l` 比較。

輸出範例：
```
Input a string: Hello OS Lab4
Original : Hello OS Lab4
Reverse  : 4baL SO olleH
Upper    : 4BAL SO OLLEH
```

### 變形自哪裡／考點
- Exercise 的「傳陣列位址、函式就地修改陣列」換成 **char 陣列**；流程同 p.11 / p.21。

### 可能性：低
簡報完全沒碰字串處理，題目重心會偏離 library；但「傳位址、原地修改」與 sort 同精神，仍有可能當成換皮題。

### 參考答案
**strlib.h**
```c
void reverse(char *s);
void to_upper(char *s);
```
**reverse.c**
```c
#include <string.h>
#include "strlib.h"

void reverse(char *s) {
    int i = 0, j = strlen(s) - 1;
    while (i < j) {
        char t = s[i]; s[i] = s[j]; s[j] = t;
        i++; j--;
    }
}
```
**upper.c**
```c
#include "strlib.h"

void to_upper(char *s) {
    for (int i = 0; s[i] != '\0'; i++)
        if (s[i] >= 'a' && s[i] <= 'z')
            s[i] = s[i] - 'a' + 'A';
}
```
**main.c**
```c
#include <stdio.h>
#include <string.h>
#include "strlib.h"

int main(void) {
    char str[100];
    printf("Input a string: ");
    fgets(str, sizeof(str), stdin);
    str[strcspn(str, "\n")] = '\0';      /* 去掉 fgets 讀進來的換行 */
    printf("Original : %s\n", str);
    reverse(str);
    printf("Reverse  : %s\n", str);
    to_upper(str);
    printf("Upper    : %s\n", str);
    return 0;
}
```
**指令序列**
```bash
gcc -c reverse.c upper.c
ar rcs strlib.a reverse.o upper.o
gcc main.c -L. strlib.a -o str_static
./str_static

gcc -c -fPIC reverse.c upper.c
gcc reverse.o upper.o -shared -o strlib.so
gcc main.c -L. strlib.so -o str_dynamic
sudo cp strlib.so /lib/strlib.so
./str_dynamic
ls -l
```

### ✅ 已驗證
```
$ echo "Hello OS Lab4" | ./str_static
Input a string: Original : Hello OS Lab4
Reverse  : 4baL SO olleH
Upper    : 4BAL SO OLLEH
$ echo "nchu 2026" | LD_LIBRARY_PATH=. ./str_dynamic
Input a string: Original : nchu 2026
Reverse  : 6202 uhcn
Upper    : 6202 UHCN
$ ls -l str_static str_dynamic strlib.a strlib.so
-rwxr-xr-x 1 root root 16208 str_dynamic
-rwxr-xr-x 1 root root 16312 str_static
-rw-r--r-- 1 root root  3042 strlib.a
-rwxr-xr-x 1 root root 15544 strlib.so
```

### 容易踩的坑
- `fgets` 會把 `\n` 一起讀進來，不去掉的話反轉後換行會跑到最前面。
- 用 `scanf("%s")` 只能讀到空白前，題目若有空白要用 `fgets`。
- 函式名不要叫 `toupper`（`<ctype.h>` 已有），改 `to_upper`。
- `strlen` 需要 `#include <string.h>`。
