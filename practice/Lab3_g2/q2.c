// Q2：只用 <unistd.h> 從 stdin(fd 0) 讀入兩個整數，輸出相加結果
#include <unistd.h>

int my_read_int(int *out) {             // 成功回傳 1，EOF 回傳 0
    char c;
    int sign = 1, got = 0;
    long long val = 0;                  // 用 long long 累加，-2147483648 才不會溢位
    // 1. 跳過空白、換行
    while (1) {
        if (read(0, &c, 1) <= 0) return 0;          // EOF
        if (c != ' ' && c != '\n' && c != '\t' && c != '\r') break;
    }
    // 2. 處理負號
    if (c == '-') {
        sign = -1;
        if (read(0, &c, 1) <= 0) return 0;
    }
    // 3. 逐字元累加數字
    while (c >= '0' && c <= '9') {
        val = val * 10 + (c - '0');
        got = 1;
        if (read(0, &c, 1) <= 0) break;
    }
    if (!got) return 0;
    *out = (int)(sign * val);
    return 1;
}

void my_print_int(long long a) {
    char buf[21];
    int i = 20;
    unsigned long long u = (a < 0) ? -(unsigned long long)a : (unsigned long long)a;
    do {
        buf[i--] = '0' + u % 10;
        u /= 10;
    } while (u > 0);
    if (a < 0) buf[i--] = '-';
    write(1, buf + i + 1, 20 - i);
}

int main() {
    int a, b;
    write(1, "Input two integers: ", 20);
    if (!my_read_int(&a) || !my_read_int(&b)) {
        write(2, "input error\n", 12);           // 錯誤訊息寫到 fd 2 (stderr)
        return 1;
    }
    write(1, "sum = ", 6);
    my_print_int((long long)a + b);             // 轉 long long 避免溢位
    write(1, "\n", 1);
    return 0;
}
