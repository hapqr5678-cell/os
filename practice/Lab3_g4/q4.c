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
