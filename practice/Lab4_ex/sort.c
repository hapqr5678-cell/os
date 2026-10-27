// LAB4: Sort with static / shared library
#include "exlib.h"

int *sort(int *arr) {
  for(int i = 0; i < 5; i++) {
    for(int j = i; j < 5; j++) {
      if(arr[j] < arr[i]) {
        int temp = arr[j];
        arr[j] = arr[i];
        arr[i] = temp;
      }
    }
  }
  return arr;
}
