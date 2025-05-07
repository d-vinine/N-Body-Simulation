#include <stdio.h>
#include <stdlib.h>
#include <time.h>

float random_test() { return random() % (10-2) + 2; }

int main(void) {
  srand(time(0));
  clock_t start, end;

  start = clock();
  int num;
  for (int i = 0; i < 1e8; i++) {
    num = random_test();
  }
  end = clock();
  double exec_time = (double)(end - start) / CLOCKS_PER_SEC;

  printf("Execution Time: %f seconds\n", exec_time);
  return 0;
}
