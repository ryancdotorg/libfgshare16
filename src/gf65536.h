#ifndef __GF65536_H_
#define __GF65536_H_
#include <stdint.h>

/* wrappers/inline impls of arithmatic used */
#define GF_MUL(A, B)    ((A) ? ((B) ? gf65536_exps[gf65536_logs[A] + gf65536_logs[B]] : 0) : 0)
#define GF_MUL_LN(A, B) ((B) ? gf65536_exps[A + gf65536_logs[B]] : 0)
#define GF_ADD(A, B)    (((A)^(B)) & 0xffff) 
#define GF_SUB(A, B)    GF_ADD(A, B)
#define GF_NEG(A)       (A)
#define GF_DIV(A, B)    gf65536_div(A, B)
#define GF_POW(A, B)    gf65536_pow(A, B)


/* log/exp lookup table storage. The original code precomputed them, but for
 * operation over GF(2**16) the tables are nearly 400KB. Computing at runtime
 * the first time the library is used takes at most tens of milliseconds.
 */
uint16_t gf65536_logs[65536];
uint16_t gf65536_exps[131070];

/* TODO move these to a .h */
void gf65536_init_tables();
uint16_t gf65536_div(uint16_t, uint16_t);
uint16_t gf65536_pow(uint16_t, uint16_t);
#endif /* __GF65536_H_ */
