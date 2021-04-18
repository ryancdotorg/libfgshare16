#ifndef __SPEED_H_
#define __SPEED_H_

#define BEGIN_SPEED_TEST(NAME) \
uint64_t NAME ## _speed_begin = speed_get_now()

#define END_SPEED_TEST(NAME, DIV) \
uint64_t NAME ## _speed_end = speed_get_now(); \
printf("%s: %luus\n", #NAME, (unsigned long)((NAME ## _speed_end - NAME ## _speed_begin) / DIV))

uint64_t speed_get_now();
#endif /* __SPEED_H_ */
