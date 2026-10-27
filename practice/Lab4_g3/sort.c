#include "exlib.h"

int *sort(int *arr) {
    for (int i = 0; i < 5; i++)
        for (int j = i + 1; j < 5; j++)
            if (arr[j] < arr[i]) {
                int t = arr[i]; arr[i] = arr[j]; arr[j] = t;
            }
    return arr;
}
