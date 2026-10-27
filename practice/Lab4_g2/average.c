#include "mylib.h"

double average(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++)
        sum += arr[i];
    return (double)sum / n;          /* 先轉 double 再除，否則整數除法會截掉小數 */
}
