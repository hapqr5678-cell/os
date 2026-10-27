// LAB4: Sort with static / shared library
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "exlib.h"

int main() {
  srand(time(NULL));
  int arr[5];
  printf("Before sort: ");
  for(int i = 0; i < 5; i++) {
    // draw again until the number is not already in the array
    int dup;
    do {
      arr[i] = rand() % 102;
      dup = 0;
      for(int j = 0; j < i; j++) {
        if(arr[j] == arr[i]) {
          dup = 1;
          break;
        }
      }
    } while(dup);
    printf("%d ", arr[i]);
  }
  sort(arr);
  printf("\nAfter sort: ");
  for(int i = 0; i < 5; i++) {
    printf("%d ", arr[i]);
  }
  printf("\n");
  return 0;
}
