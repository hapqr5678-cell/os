#include <stdio.h>
#include "lib.h"

int main(void) {
    int a, b;
    printf("Input two integers: ");
    scanf("%d %d", &a, &b);
    printf("%d + %d = %d\n", a, b, add(a, b));
    printf("%d - %d = %d\n", a, b, sub(a, b));
    printf("%d * %d = %d\n", a, b, mul(a, b));
    if (b != 0)
        printf("%d / %d = %.2f\n", a, b, divide(a, b));
    else
        printf("%d / %d = undefined (divide by zero)\n", a, b);
    return 0;
}
