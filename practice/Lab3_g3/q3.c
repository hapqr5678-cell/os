// Q3：只用 system call (open/read/write/close) 複製檔案
// 用法：./mycp <src> <dst>
#include <unistd.h>
#include <fcntl.h>      // O_RDONLY, O_CREAT ... 這些 flag 定義在這裡

void my_print_int(long long a) {
    char buf[21];
    int i = 20;
    unsigned long long u = (a < 0) ? -(unsigned long long)a : (unsigned long long)a;
    do { buf[i--] = '0' + u % 10; u /= 10; } while (u > 0);
    if (a < 0) buf[i--] = '-';
    write(1, buf + i + 1, 20 - i);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        write(2, "usage: ./mycp <src> <dst>\n", 26);
        return 1;
    }
    int src = open(argv[1], O_RDONLY);
    if (src == -1) {
        write(2, "open src failed\n", 16);
        return 1;
    }
    int dst = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (dst == -1) {
        write(2, "open dst failed\n", 16);
        close(src);
        return 1;
    }

    char buffer[4096];
    ssize_t n;
    long long total = 0;
    while ((n = read(src, buffer, sizeof(buffer))) > 0) {   // 0 = EOF
        write(dst, buffer, n);           // 寫 n 個，不是 sizeof(buffer)！
        total += n;
    }

    close(src);
    close(dst);
    write(1, "copied ", 7);
    my_print_int(total);
    write(1, " bytes\n", 7);
    return 0;
}
