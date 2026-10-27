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
