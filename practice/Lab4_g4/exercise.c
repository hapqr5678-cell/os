#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "exlib.h"

int main(void) {
    int arr[5], cnt = 0;
    srand(time(NULL));
    while (cnt < 5) {
        int x = rand() % 100 + 1, dup = 0;
        for (int k = 0; k < cnt; k++)
            if (arr[k] == x) { dup = 1; break; }
        if (!dup) arr[cnt++] = x;
    }
    printf("Before sort: ");
    for (int i = 0; i < 5; i++) printf("%d ", arr[i]);
    printf("\n");
    sort(arr);
    printf("After sort: ");
    for (int i = 0; i < 5; i++) printf("%d ", arr[i]);
    printf("\n");
    return 0;
}
