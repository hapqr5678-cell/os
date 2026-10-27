#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mylib.h"

int main(void) {
    int arr[5], cnt = 0;
    srand(time(NULL));
    while (cnt < 5) {
        int x = rand() % 100 + 1, dup = 0;
        for (int k = 0; k < cnt; k++)
            if (arr[k] == x) { dup = 1; break; }
        if (!dup) arr[cnt++] = x;
    }
    printf("Array: ");
    for (int i = 0; i < 5; i++) printf("%d ", arr[i]);
    printf("\nMax = %d\n", max(arr, 5));
    printf("Min = %d\n", min(arr, 5));
    printf("Average = %.2f\n", average(arr, 5));
    return 0;
}
