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
