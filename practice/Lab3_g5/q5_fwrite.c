// Q5-b：用 fwrite() 一次寫 1 byte，共寫 1MB，量時間
#include <stdio.h>
#include <time.h>

#define TOTAL (1024 * 1024)

int main() {
    struct timespec t1, t2;
    char c = 'A';
    FILE *fp = fopen("out_fwrite.txt", "w");
    if (fp == NULL) { perror("fopen"); return 1; }

    clock_gettime(CLOCK_MONOTONIC, &t1);
    for (int i = 0; i < TOTAL; i++)
        fwrite(&c, 1, 1, fp);           // 先放進 user-space buffer，滿了才 write()
    fclose(fp);                         // fclose 會 flush 剩下的資料
    clock_gettime(CLOCK_MONOTONIC, &t2);

    printf("%f\n", (t2.tv_sec - t1.tv_sec) + (t2.tv_nsec - t1.tv_nsec) / 1e9);
    return 0;
}
