#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <check.h>

#include "gf65536.h"

/*
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
*/

/* setup fixture */
void test_gf65536_setup() {
  gf65536_init_tables();
  int i;

  uint8_t seen_exps[65536];
  uint8_t seen_logs[65536];
  for (i = 0; i < 65536; ++i) {
    seen_exps[i] = seen_logs[i] = 0;
  }

  for (i = 0; i <= 65534; ++i) {
    ck_assert_int_eq(gf65536_exps[i+65535], gf65536_exps[i]);
    ck_assert_int_eq(gf65536_logs[gf65536_exps[i]], i);
    seen_exps[gf65536_exps[i]] = 1;
  }

  for (i = 1; i <= 65535; ++i) {
    ck_assert_int_eq(gf65536_exps[gf65536_logs[i]], i);
    seen_logs[gf65536_logs[i]] = 1;
  }

  int count_exps = 0;
  int count_logs = 0;
  for (i = 0; i < 65536; ++i) {
    count_exps += seen_exps[i];
    count_logs += seen_logs[i];
  }
  ck_assert_int_eq(count_exps, 65535);
  ck_assert_int_eq(count_logs, 65535);
}

void test_gf65536_teardown() {
  ;
}

/* Addition over a binary field is xor */
#define TEST_GF65536_ADD_SAME(V) \
START_TEST (test_gf65536_add_same_ ## V) { \
  ck_assert_int_eq(GF_ADD(V, V), 0); \
} \
END_TEST

TEST_GF65536_ADD_SAME(0)
TEST_GF65536_ADD_SAME(5)
TEST_GF65536_ADD_SAME(9)
TEST_GF65536_ADD_SAME(81)
TEST_GF65536_ADD_SAME(729)
TEST_GF65536_ADD_SAME(6561)
TEST_GF65536_ADD_SAME(59049)
TEST_GF65536_ADD_SAME(65535)

#define TEST_GF65536_ADD_COMM(A, B) \
START_TEST (test_gf65536_add_comm_ ## A ## _ ## B) { \
  ck_assert_int_eq(GF_ADD(A, B), GF_ADD(B, A)); \
} \
END_TEST

TEST_GF65536_ADD_COMM(0, 1)
TEST_GF65536_ADD_COMM(0, 123)
TEST_GF65536_ADD_COMM(0, 256)
TEST_GF65536_ADD_COMM(0, 16384)
TEST_GF65536_ADD_COMM(0, 31337)
TEST_GF65536_ADD_COMM(0, 65535)

TEST_GF65536_ADD_COMM(1, 123)
TEST_GF65536_ADD_COMM(1, 256)
TEST_GF65536_ADD_COMM(1, 16384)
TEST_GF65536_ADD_COMM(1, 31337)
TEST_GF65536_ADD_COMM(1, 65535)

TEST_GF65536_ADD_COMM(256, 123)
TEST_GF65536_ADD_COMM(256, 16384)
TEST_GF65536_ADD_COMM(256, 31337)
TEST_GF65536_ADD_COMM(256, 65535)

TEST_GF65536_ADD_COMM(31337, 16384)
TEST_GF65536_ADD_COMM(31337, 65535)

// Associative
#define TEST_GF65536_ADD_ASSO(A, B, C) \
START_TEST (test_gf65536_add_asso_ ## A ## _ ## B ## _ ## C) { \
  ck_assert_int_eq(GF_ADD(GF_ADD(A, B), C), GF_ADD(A, GF_ADD(B, C))); \
  ck_assert_int_eq(GF_ADD(GF_ADD(A, C), B), GF_ADD(A, GF_ADD(B, C))); \
} \
END_TEST

/* mostly random, first three are modified */
TEST_GF65536_ADD_ASSO(0,     0,     29947)
TEST_GF65536_ADD_ASSO(0,     2962,  36497)
TEST_GF65536_ADD_ASSO(1,     256,   32768)
TEST_GF65536_ADD_ASSO(852,   12807, 52728)
TEST_GF65536_ADD_ASSO(1582,  47073, 48981)
TEST_GF65536_ADD_ASSO(3118,  25765, 56376)
TEST_GF65536_ADD_ASSO(7594,  19936, 59876)
TEST_GF65536_ADD_ASSO(11755, 29095, 58900)

#define TEST_GF65536_MUL_COMM(A, B) \
START_TEST (test_gf65536_mul_comm_ ## A ## _ ## B) { \
  ck_assert_int_eq(GF_MUL(A, B), GF_MUL(B, A)); \
} \
END_TEST

TEST_GF65536_MUL_COMM(0, 1)
TEST_GF65536_MUL_COMM(0, 123)
TEST_GF65536_MUL_COMM(0, 256)
TEST_GF65536_MUL_COMM(0, 16384)
TEST_GF65536_MUL_COMM(0, 31337)
TEST_GF65536_MUL_COMM(0, 65535)

TEST_GF65536_MUL_COMM(1, 123)
TEST_GF65536_MUL_COMM(1, 256)
TEST_GF65536_MUL_COMM(1, 16384)
TEST_GF65536_MUL_COMM(1, 31337)
TEST_GF65536_MUL_COMM(1, 65535)

TEST_GF65536_MUL_COMM(256, 123)
TEST_GF65536_MUL_COMM(256, 16384)
TEST_GF65536_MUL_COMM(256, 31337)
TEST_GF65536_MUL_COMM(256, 65535)

TEST_GF65536_MUL_COMM(31337, 16384)
TEST_GF65536_MUL_COMM(31337, 65535)

// Associative
#define TEST_GF65536_MUL_ASSO(A, B, C) \
START_TEST (test_gf65536_mul_asso_ ## A ## _ ## B ## _ ## C) { \
  ck_assert_int_eq(GF_MUL(GF_MUL(A, B), C), GF_MUL(A, GF_MUL(B, C))); \
  ck_assert_int_eq(GF_MUL(GF_MUL(A, C), B), GF_MUL(A, GF_MUL(B, C))); \
} \
END_TEST

/* mostly random, first three are modified */
TEST_GF65536_MUL_ASSO(0,     0,     29947)
TEST_GF65536_MUL_ASSO(0,     2962,  36497)
TEST_GF65536_MUL_ASSO(1,     256,   32768)
TEST_GF65536_MUL_ASSO(852,   12807, 52728)
TEST_GF65536_MUL_ASSO(1582,  47073, 48981)
TEST_GF65536_MUL_ASSO(3118,  25765, 56376)
TEST_GF65536_MUL_ASSO(7594,  19936, 59876)
TEST_GF65536_MUL_ASSO(11755, 29095, 58900)

#define TEST_GF65536_DISTRIB(A, B, C) \
START_TEST (test_gf65536_distrib_ ## A ## _ ## B ## _ ## C) { \
  ck_assert_int_eq( GF_MUL(GF_ADD(A, B), C), GF_ADD(GF_MUL(A, C), GF_MUL(B, C)) ); \
} \
END_TEST

/* mostly random, first three are modified */
TEST_GF65536_DISTRIB(0,     0,     29947)
TEST_GF65536_DISTRIB(0,     2962,  36497)
TEST_GF65536_DISTRIB(1,     256,   32768)
TEST_GF65536_DISTRIB(852,   12807, 52728)
TEST_GF65536_DISTRIB(1582,  47073, 48981)
TEST_GF65536_DISTRIB(3118,  25765, 56376)
TEST_GF65536_DISTRIB(7594,  19936, 59876)
TEST_GF65536_DISTRIB(11755, 29095, 58900)

Suite *
gf65536_suite (void) {
  Suite *s = suite_create ("gf65536");

  TCase *tc_add_same = tcase_create ("Add Same");
  tcase_add_checked_fixture (tc_add_same, test_gf65536_setup, test_gf65536_teardown);
  tcase_add_test (tc_add_same, test_gf65536_add_same_0);
  tcase_add_test (tc_add_same, test_gf65536_add_same_5);
  tcase_add_test (tc_add_same, test_gf65536_add_same_9);
  tcase_add_test (tc_add_same, test_gf65536_add_same_81);
  tcase_add_test (tc_add_same, test_gf65536_add_same_729);
  tcase_add_test (tc_add_same, test_gf65536_add_same_6561);
  tcase_add_test (tc_add_same, test_gf65536_add_same_59049);
  tcase_add_test (tc_add_same, test_gf65536_add_same_65535);
  suite_add_tcase (s, tc_add_same);

  TCase *tc_add_comm = tcase_create ("Add Commutative");
  tcase_add_checked_fixture (tc_add_comm, test_gf65536_setup, test_gf65536_teardown);
  tcase_add_test (tc_add_comm, test_gf65536_add_comm_0_1);
  tcase_add_test (tc_add_comm, test_gf65536_add_comm_0_123);
  tcase_add_test (tc_add_comm, test_gf65536_add_comm_0_256);
  tcase_add_test (tc_add_comm, test_gf65536_add_comm_0_16384);
  tcase_add_test (tc_add_comm, test_gf65536_add_comm_0_31337);
  tcase_add_test (tc_add_comm, test_gf65536_add_comm_0_65535);

  tcase_add_test (tc_add_comm, test_gf65536_add_comm_1_123);
  tcase_add_test (tc_add_comm, test_gf65536_add_comm_1_256);
  tcase_add_test (tc_add_comm, test_gf65536_add_comm_1_16384);
  tcase_add_test (tc_add_comm, test_gf65536_add_comm_1_31337);
  tcase_add_test (tc_add_comm, test_gf65536_add_comm_1_65535);

  tcase_add_test (tc_add_comm, test_gf65536_add_comm_256_123);
  tcase_add_test (tc_add_comm, test_gf65536_add_comm_256_16384);
  tcase_add_test (tc_add_comm, test_gf65536_add_comm_256_31337);
  tcase_add_test (tc_add_comm, test_gf65536_add_comm_256_65535);

  tcase_add_test (tc_add_comm, test_gf65536_add_comm_31337_16384);
  tcase_add_test (tc_add_comm, test_gf65536_add_comm_31337_65535);
  suite_add_tcase (s, tc_add_comm);

  TCase *tc_add_asso = tcase_create ("Add Associtive");
  tcase_add_test (tc_add_asso, test_gf65536_add_asso_0_0_29947);
  tcase_add_test (tc_add_asso, test_gf65536_add_asso_0_2962_36497);
  tcase_add_test (tc_add_asso, test_gf65536_add_asso_1_256_32768);
  tcase_add_test (tc_add_asso, test_gf65536_add_asso_852_12807_52728);
  tcase_add_test (tc_add_asso, test_gf65536_add_asso_1582_47073_48981);
  tcase_add_test (tc_add_asso, test_gf65536_add_asso_3118_25765_56376);
  tcase_add_test (tc_add_asso, test_gf65536_add_asso_7594_19936_59876);
  tcase_add_test (tc_add_asso, test_gf65536_add_asso_11755_29095_58900);
  suite_add_tcase (s, tc_add_asso);

  TCase *tc_mul_comm = tcase_create ("Multiply Commutative");
  tcase_add_checked_fixture (tc_mul_comm, test_gf65536_setup, test_gf65536_teardown);
  tcase_add_test (tc_mul_comm, test_gf65536_mul_comm_0_1);
  tcase_add_test (tc_mul_comm, test_gf65536_mul_comm_0_123);
  tcase_add_test (tc_mul_comm, test_gf65536_mul_comm_0_256);
  tcase_add_test (tc_mul_comm, test_gf65536_mul_comm_0_16384);
  tcase_add_test (tc_mul_comm, test_gf65536_mul_comm_0_31337);
  tcase_add_test (tc_mul_comm, test_gf65536_mul_comm_0_65535);

  tcase_add_test (tc_mul_comm, test_gf65536_mul_comm_1_123);
  tcase_add_test (tc_mul_comm, test_gf65536_mul_comm_1_256);
  tcase_add_test (tc_mul_comm, test_gf65536_mul_comm_1_16384);
  tcase_add_test (tc_mul_comm, test_gf65536_mul_comm_1_31337);
  tcase_add_test (tc_mul_comm, test_gf65536_mul_comm_1_65535);

  tcase_add_test (tc_mul_comm, test_gf65536_mul_comm_256_123);
  tcase_add_test (tc_mul_comm, test_gf65536_mul_comm_256_16384);
  tcase_add_test (tc_mul_comm, test_gf65536_mul_comm_256_31337);
  tcase_add_test (tc_mul_comm, test_gf65536_mul_comm_256_65535);

  tcase_add_test (tc_mul_comm, test_gf65536_mul_comm_31337_16384);
  tcase_add_test (tc_mul_comm, test_gf65536_mul_comm_31337_65535);
  suite_add_tcase (s, tc_mul_comm);

  TCase *tc_mul_asso = tcase_create ("Multiply Associtive");
  tcase_add_test (tc_mul_asso, test_gf65536_mul_asso_0_0_29947);
  tcase_add_test (tc_mul_asso, test_gf65536_mul_asso_0_2962_36497);
  tcase_add_test (tc_mul_asso, test_gf65536_mul_asso_1_256_32768);
  tcase_add_test (tc_mul_asso, test_gf65536_mul_asso_852_12807_52728);
  tcase_add_test (tc_mul_asso, test_gf65536_mul_asso_1582_47073_48981);
  tcase_add_test (tc_mul_asso, test_gf65536_mul_asso_3118_25765_56376);
  tcase_add_test (tc_mul_asso, test_gf65536_mul_asso_7594_19936_59876);
  tcase_add_test (tc_mul_asso, test_gf65536_mul_asso_11755_29095_58900);
  suite_add_tcase (s, tc_mul_asso);

  TCase *tc_distrib = tcase_create ("Distributive");
  tcase_add_test (tc_distrib, test_gf65536_distrib_0_0_29947);
  tcase_add_test (tc_distrib, test_gf65536_distrib_0_2962_36497);
  tcase_add_test (tc_distrib, test_gf65536_distrib_1_256_32768);
  tcase_add_test (tc_distrib, test_gf65536_distrib_852_12807_52728);
  tcase_add_test (tc_distrib, test_gf65536_distrib_1582_47073_48981);
  tcase_add_test (tc_distrib, test_gf65536_distrib_3118_25765_56376);
  tcase_add_test (tc_distrib, test_gf65536_distrib_7594_19936_59876);
  tcase_add_test (tc_distrib, test_gf65536_distrib_11755_29095_58900);
  suite_add_tcase (s, tc_distrib);
  return s;
}

int main() {
  /* tests */
  int number_failed;
  Suite *s = gf65536_suite();
  SRunner *sr = srunner_create(s);
  srunner_run_all(sr, CK_ENV);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;

  return 0;
}
/* vim: ts=2 sw=2 et ai si bg=dark
*/
