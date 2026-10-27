#include <stdio.h>
#include "exlib.h"

/* v2：改成由大到小 */
int *sort(int *arr) {
    printf("[exlib v2] descending\n");
    for (int i = 0; i < 5; i++)
        for (int j = i + 1; j < 5; j++)
            if (arr[j] > arr[i]) {
                int t = arr[i]; arr[i] = arr[j]; arr[j] = t;
            }
    return arr;
}
