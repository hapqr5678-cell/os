# Lab2_g2

> 來源:review/predict/Lab2_predict.md

### 題 2(可能性:**高**)SYSCALL_DEFINE2:兩個參數相加,並回傳結果

**題目**:改寫 hello syscall,用 `SYSCALL_DEFINE2()` 接收兩個整數 a、b。在 kernel log 印出 `a + b = 結果`,並把結果**回傳**給 user program 印出。執行 `syscall(462, 3, 5)`。

**預期輸出**
```
$ ./test
return = 8
$ sudo dmesg | tail -1
[  xxx.xxxxxx] 3 + 5 = 8
```

- **變形來源**:p.42 的 `SYSCALL_DEFINE1` 推廣成 `DEFINE2`,加上 p.38 備註「syscall() 成功回傳 0 或正數」(回傳值)、p.37 的 `printf("%ld\n", syscall(...))`。
- **為什麼可能性高**:符合共通出題邏輯「把核心機制換個小題目再用一次」,也就是「1 個參數 → 2 個參數」,跟其他 lab 的 add/sub 題型如出一轍。而且只要改 hello.c,時間來得及。
- **考的觀念**:多參數巨集「型別, 名稱」成對出現;回傳值會變成 user 端 `syscall()` 的回傳值。

**參考答案**(⚠️ kernel 端無法在此環境驗證)

`~/linux/hello/hello.c`
```c
#include <linux/kernel.h>
#include <linux/syscalls.h>

SYSCALL_DEFINE2(hello, int, a, int, b)
{
    int sum = a + b;

    pr_info("%d + %d = %d\n", a, b, sum);
    return sum;
}
```

其他檔案

| 檔案 | 內容 |
|---|---|
| `include/linux/syscalls.h` | `asmlinkage long sys_hello(int a, int b);` |
| `hello/Makefile`、`Kbuild`、`syscall_64.tbl` | 不用動 |

user 端 `test.c`(✅ 已通過 gcc 檢查)
```c
#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
int main(void) {
    long r = syscall(462, 3, 5);
    printf("return = %ld\n", r);
    return 0;
}
```

**容易踩的坑**
- 寫成 `SYSCALL_DEFINE2(hello, int a, int b)`:錯。數字 2 代表 2 組「型別, 名稱」,全部用逗號隔開。
- `DEFINEn` 的 n 要和參數組數一致。寫 `SYSCALL_DEFINE1(hello, int, a, int, b)` 會編譯錯誤。
- 如果改成減法而且結果是負數(例如 3−5 = −2):user 端會看到 **-1**,errno 被設成 2。原因是 libc 的 `syscall()` 把 -1~-4095 之間的回傳值當成錯誤碼。
  - 這是延伸觀念,簡報沒教,但跟 p.38「失敗回傳 -1」有關。
  - 如果題目要做減法,答案要從 dmesg 看,不要靠回傳值。
- user 端用 `%d` 印 `long`:會有警告,要用 `%ld`。
