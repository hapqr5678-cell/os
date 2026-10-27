// LAB3: my_print (print integers with unistd.h only)
#include <unistd.h>

void my_print(int a) {
  char buffer[13];  // '-' + 10 digits + '\n', with room to spare
  int i = 11;
  buffer[12] = '\n';

  // use unsigned so that -2147483648 does not overflow when negated
  unsigned int u = (a < 0) ? -(unsigned int)a : (unsigned int)a;
  do {
    buffer[i--] = '0' + u % 10;
    u = u / 10;
  } while(u > 0);
  if(a < 0) {
    buffer[i--] = '-';
  }
  write(1, buffer + i + 1, 12 - i);
}

int main() {
  my_print(0);
  my_print(100);
  my_print(2147483647);
  my_print(-2147483648);
  return 0;
}
