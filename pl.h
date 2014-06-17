
#ifndef PL_H_
#define PL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define PL_FALSE 0
#define PL_TRUE 1
#define PL_UNDEFINED 2
#define PL_UNUSED 3
#define PL_MAX_VARS 128

typedef struct {
    char variable;
	uint8_t negated;
} PL_LITERAL;

typedef struct {
    uint32_t count;
	PL_LITERAL *literals;
} PL_DISJUNCTION;

typedef struct {
    uint32_t count;
	PL_DISJUNCTION *disjunctions;
} PL_CONJUNCTION;

typedef struct {
    uint8_t dictionary[PL_MAX_VARS];
} PL_INTERPRETATION;

PL_CONJUNCTION* pl_parse(char *str);
uint8_t pl_is_disjunction_satisfied(PL_DISJUNCTION* d, PL_INTERPRETATION *i);
uint8_t pl_is_satisfied(PL_CONJUNCTION *conj, PL_INTERPRETATION *inter);
PL_INTERPRETATION* pl_brute_force(PL_CONJUNCTION *conjunction);
PL_INTERPRETATION* pl_create_interpretation(PL_CONJUNCTION *c, uint8_t def);
void pl_print_interpretation(PL_INTERPRETATION *inter);

#ifdef __cplusplus
}
#endif

#endif

