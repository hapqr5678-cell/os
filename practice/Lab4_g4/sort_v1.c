#include <stdio.h>
#include "exlib.h"

/* v1：由小到大 */
int *sort(int *arr) {
    printf("[exlib v1] ascending\n");
    for (int i = 0; i < 5; i++)
        for (int j = i + 1; j < 5; j++)
            if (arr[j] < arr[i]) {
                int t = arr[i]; arr[i] = arr[j]; arr[j] = t;
            }
    return arr;
}
