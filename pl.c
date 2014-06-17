
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pl.h"


/**
 * Returns true if the character is a letter.
 *
 * @param c The character
 * @return True if a letter.
 */
static uint32_t is_letter(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}


/**
 * Counts the number of tokens in string str separated by delimiter delim.
 *
 * @param str The input string.
 * @param delim The token delimiter.
 * @return The number of tokens.
 */
static uint32_t count_tokens(char *str, char delim)
{
    uint32_t count = 1;
    char *tmp = str;
    while (*tmp) {
        if (*tmp == delim) {
            count++;
        }
        tmp++;
    }
    return count;
}


/**
 * public domain strtok_r() by Charlie Gordon
 *
 *   from comp.lang.c  9/14/2007
 *
 *      http://groups.google.com/group/comp.lang.c/msg/2ab1ecbb86646684
 *
 *     (Declaration that it's public domain):
 *      http://groups.google.com/group/comp.lang.c/msg/7c7b39328fefab9c
 */
static char* strtok_r(char *str, const char *delim, char **nextp)
{
    char *ret;

    if (str == NULL) {
        str = *nextp;
    }

    str += strspn(str, delim);

    if (*str == '\0') {
        return NULL;
    }

    ret = str;

    str += strcspn(str, delim);

    if (*str) {
        *str++ = '\0';
    }

    *nextp = str;

    return ret;
}


/**
 * Parses a propositional logic literal.
 *
 * @param str The input string.
 * @param literal The output literal.
 */
static void parse_literal(char *str, PL_LITERAL *literal)
{
    literal->negated = 0;
    char *tmp = str;
    while (*tmp) {
        if (*tmp == '~') {
            literal->negated = 1;
        } else if (is_letter(*tmp)) {
            literal->variable = *tmp;
            break;
        }
        tmp++;
    }
}


/**
 * Parses a propositional logic disjunctions.
 *
 * @param str The input string.
 * @param result The output disjunction.
 */
static void parse_disjunction(char *str, PL_DISJUNCTION *result)
{
    result->count = count_tokens(str, 'v');
    result->literals = calloc(result->count, sizeof(PL_LITERAL));
    
    char *end_str = NULL;
    char* token = strtok_r(str, "v", &end_str);
    uint32_t index = 0;
    while (token) {
        parse_literal(token, &result->literals[index]);
        token = strtok_r(NULL, "v", &end_str);
        index++;
    }
}


/**
 * Parses a propositional logic conjunction.
 *
 * @param str The input string.
 * @return A propositional logic conjunction.
 */
PL_CONJUNCTION* pl_parse(char *str)
{
    PL_CONJUNCTION* result = calloc(1, sizeof(PL_CONJUNCTION));
    result->count = count_tokens(str, '^');
    result->disjunctions = calloc(result->count, sizeof(PL_DISJUNCTION));
    
    char *end_str = NULL;
    char* token = strtok_r(str, "^", &end_str);
    uint32_t index = 0;
    while (token) {
        parse_disjunction(token, &result->disjunctions[index]);
        token = strtok_r(NULL, "^", &end_str);
        index++;
    }

    return result;
}


/**
 * Determines if a literal is satisfied by an interpretation.
 *
 * @param lit The literal.
 * @param inter The interpretation.
 * @return PL_TRUE if satisfied;
 *         PL_FALSE if not satisfied;
 *         PL_UNDEFINED if unsolvable.
 */
static uint8_t lit_satisfied(PL_LITERAL *lit, PL_INTERPRETATION *inter)
{
    uint8_t result = inter->dictionary[(uint8_t)lit->variable];
    
    if (result == PL_UNDEFINED) {
        return PL_UNDEFINED;
    }
    
    if (lit->negated) {
        return result == PL_TRUE ? PL_FALSE : PL_TRUE;
    }
    
    return result;
}


/**
 * Determines if a disjunction is satisfied by an interpretation.
 *
 * @param disj The disjunction.
 * @param inter The interpretation.
 * @return PL_TRUE if satisfied;
 *         PL_FALSE if not satisfied;
 *         PL_UNDEFINED if unsolvable.
 */
static uint8_t disj_satisfied(PL_DISJUNCTION* disj, PL_INTERPRETATION *inter)
{
    for (uint32_t i = 0; i < disj->count; i++) {
        PL_LITERAL *literal = &disj->literals[i];
        uint8_t result = lit_satisfied(literal, inter);
        if (result != PL_FALSE) {
            return result;
        }
    }
    
    return PL_FALSE;
}


/**
 * Determines if a conjunction is satisfied by an interpretation.
 *
 * @param conj The conjunction.
 * @param inter The interpretation.
 * @return PL_TRUE if satisfied;
 *         PL_FALSE if not satisfied;
 *         PL_UNDEFINED if unsolvable.
 */
uint8_t pl_is_satisfied(PL_CONJUNCTION *conj, PL_INTERPRETATION *inter)
{
    for (uint32_t i = 0; i < conj->count; i++) {
        PL_DISJUNCTION *disjunction = &conj->disjunctions[i];
        uint8_t result = disj_satisfied(disjunction, inter);
        if (result != PL_TRUE) {
            return result;
        }
    }
    
    return PL_TRUE;
}


/**
 * Creates a default interpretation for the conjunction.
 * All used variables are set to PL_FALSE.
 * All unused variables are set to PL_UNDEFINED.
 *
 * @param conj The conjunction.
 * @return A default interpretation.
 */
static PL_INTERPRETATION* create_interpretation(PL_CONJUNCTION *conjunction)
{
    PL_INTERPRETATION* result = calloc(1, sizeof(PL_INTERPRETATION));

    for (uint32_t i = 0; i < PL_MAX_VARS; i++) {
        result->dictionary[i] = PL_UNDEFINED;
    }
    
    for (uint32_t i = 0; i < conjunction->count; i++) {
        PL_DISJUNCTION* disjunction = &conjunction->disjunctions[i];
        for (uint32_t j = 0; j < disjunction->count; j++) {
            PL_LITERAL* literal = &disjunction->literals[j];
            result->dictionary[(uint8_t)literal->variable] = PL_FALSE;
        }
    }
    
    return result;
}


/**
 * Prints the interpretation to stdout.
 *
 * @param inter The interpretation.
 */
void pl_print_interpretation(PL_INTERPRETATION *inter)
{
    if (inter == NULL) {
        printf("No solution\n");
        return;
    }

    for (uint32_t i = 0; i < PL_MAX_VARS; i++) {
        if (inter->dictionary[i] == PL_UNDEFINED) {
            continue;
        }
        if (is_letter(i)) {
            printf("%c = ", (char)i);
            switch (inter->dictionary[i]) {
            case PL_FALSE:
                printf("False\n");
                break;
                
            case PL_TRUE:
                printf("True\n");
                break;
            }
        }
    }
}


/**
 * Steps the brute force solution.
 * Imagine that each *used* variable is a bit in a twos compliment number.
 * We simply count up in binary.
 * If there is an overflow (i.e., all bits are true), then we quit.
 *
 * @param conj The conjunction.
 * @param inter The interpretation.
 * @return True if exhausted; false if more possible solutions exist.
 */
static uint8_t step_brute_force(PL_CONJUNCTION *conj, PL_INTERPRETATION *inter)
{
    for (uint32_t i = 0; i < PL_MAX_VARS; i++) {
        if (inter->dictionary[i] == PL_UNDEFINED) {
            // Unused variable -> ignore
            continue;
        }
        if (inter->dictionary[i] == PL_TRUE) {
            // Variable is true -> flip to false and continue.
            inter->dictionary[i] = PL_FALSE;
        } else {
            inter->dictionary[i] = PL_TRUE;
            // First false variable -> flip to true and return.
            return 0;
        }
    }
    return 1;
}


/**
 * Attempts to solve the conjunction using brute force.
 *
 * @param conjunction The conjunction.
 * @return The first interpretation that solves the conjunction.
 */
PL_INTERPRETATION* pl_brute_force(PL_CONJUNCTION *conjunction)
{
    PL_INTERPRETATION* inter = create_interpretation(conjunction);
    while (pl_is_satisfied(conjunction, inter) != PL_TRUE) {
        if (step_brute_force(conjunction, inter)) {
            // We exhausted all possible interpretations.
            // The conjunction is unsolvable.
            free(inter);
            return NULL;
        }
    }
    return inter;
}
