#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "gf65536.h"

int main() {
  gf65536_init_tables();
  srand(time(NULL));

  printf("%u\n", GF_MUL(5, 5));
  printf("%u\n", GF_DIV(GF_MUL(17, 5), 5));
  printf("%u\n", GF_POW(5, 5));
  printf("%u\n", GF_POW(3, 2));
  printf("%u\n", GF_POW(2, 5));
  printf("%u\n", GF_POW(2, 15));
  printf("%u\n", GF_POW(2, 16));
  printf("%u\n", GF_POW(2, 17));
  printf("%u\n", GF_POW(2, 4094));
  printf("%u\n", GF_POW(2, 4095));
  printf("%u\n", GF_POW(7, 1337));
  printf("%u\n", GF_POW(2, 65534));
  printf("%u\n", GF_POW(2, 65535));
  printf("%u\n", GF_POW(7, 31337));
  return 0;
}
