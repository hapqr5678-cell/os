#include "exlib.h"

void sort_desc(int *arr, int n) {
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - 1 - i; j++)
            if (arr[j] < arr[j + 1]) {      /* 前面比後面小就交換 → 由大到小 */
                int t = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = t;
            }
}
