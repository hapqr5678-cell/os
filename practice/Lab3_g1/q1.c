// Q1：只用 <unistd.h> 印出字串與整數陣列（含負數）
#include <unistd.h>

int my_strlen(const char *s) {
    int n = 0;
    while (s[n] != '\0') n++;
    return n;
}

void my_puts(const char *s) {           // 印字串（不自動換行）
    write(1, s, my_strlen(s));
}

void my_print_int(int a) {              // 印整數（不換行）
    char buf[12];                       // '-' + 10 位數 + 1 備用
    int i = 11;
    unsigned int u = (a < 0) ? -(unsigned int)a : (unsigned int)a;
    do {
        buf[i--] = '0' + u % 10;
        u /= 10;
    } while (u > 0);
    if (a < 0) buf[i--] = '-';
    write(1, buf + i + 1, 11 - i);
}

int main() {
    int arr[] = {3, -15, 0, 2147483647, -2147483648};
    int n = sizeof(arr) / sizeof(arr[0]);

    my_puts("arr = ");
    for (int k = 0; k < n; k++) {
        my_print_int(arr[k]);
        my_puts(k == n - 1 ? "\n" : " ");
    }
    my_puts("n = ");
    my_print_int(n);
    my_puts("\n");
    return 0;
}
