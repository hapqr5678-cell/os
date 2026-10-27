#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "exlib.h"

#define N 10

int main(void) {
    int arr[N], cnt = 0;
    srand(time(NULL));
    while (cnt < N) {                      /* 產生 N 個不重複 1~100 */
        int x = rand() % 100 + 1, dup = 0;
        for (int k = 0; k < cnt; k++)
            if (arr[k] == x) { dup = 1; break; }
        if (!dup) arr[cnt++] = x;
    }
    printf("Before sort: ");
    for (int i = 0; i < N; i++) printf("%d ", arr[i]);
    sort_desc(arr, N);                     /* 傳陣列位址 + 長度 */
    printf("\nAfter sort: ");
    for (int i = 0; i < N; i++) printf("%d ", arr[i]);
    printf("\n");
    return 0;
}
