
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dpll.h"


typedef struct {
    PL_CONJUNCTION *conjunction;
    PL_INTERPRETATION *interpretation;
    char variables[PL_MAX_VARS];
    int variables_count;
} PL_DPLL;


/**
 * Searches for a "unit symbol", which is a literal inside a disjunction
 * that must be set for the disjunction to be satisfied.
 *
 * @param dpll The DPLL working set.
 * @return A unit symbol if found; null otherwise.
 */
static PL_LITERAL* dpll_find_unit(PL_DPLL *dpll)
{
    PL_CONJUNCTION* conj = dpll->conjunction;
    PL_INTERPRETATION* inter = dpll->interpretation;

    for (uint32_t i = 0; i < conj->count; i++) {
        PL_DISJUNCTION *disjunction = &conj->disjunctions[i];
        uint8_t result = pl_is_disjunction_satisfied(disjunction, inter);
        if (result == PL_TRUE) {
            continue;
        }

        PL_LITERAL *match = NULL;
        uint32_t unset = 0;
        for (uint32_t j = 0; j < disjunction->count; j++) {
            PL_LITERAL* literal = &disjunction->literals[j];
            if (inter->dictionary[(uint8_t)literal->variable] == PL_UNDEFINED) {
                unset++;
                match = literal;
            }
        }

        if (match != NULL && unset == 1) {
            return match;
        }
    }

    return NULL;
}


/**
 * Searches for a "pure symbol", which is a literal that must have the same
 * value in all disjunctions in the entire conjunction.
 *
 * @param dpll The DPLL working set.
 * @param variable The variable.
 * @return A pure symbol if found; null otherwise.
 */
static PL_LITERAL* dpll_find_pure_for_variable(PL_DPLL *dpll, char variable)
{
    PL_CONJUNCTION* conj = dpll->conjunction;
    PL_INTERPRETATION* inter = dpll->interpretation;
    PL_LITERAL *match = NULL;

    for (uint32_t i = 0; i < conj->count; i++) {
        PL_DISJUNCTION *disjunction = &conj->disjunctions[i];
        uint8_t result = pl_is_disjunction_satisfied(disjunction, inter);
        if (result == PL_TRUE) {
            continue;
        }

        for (uint32_t j = 0; j < disjunction->count; j++) {
            PL_LITERAL* literal = &disjunction->literals[j];
            if (literal->variable != variable) {
                continue;
            }

            if (match == NULL) {
                match = literal;
            } else if (match->negated != literal->negated) {
                return NULL;
            }
        }
    }

    return match;
}


/**
 * Searches for a "pure symbol", which is a literal that must have the same
 * value in all disjunctions in the entire conjunction.
 *
 * @param dpll The DPLL working set.
 * @return A pure symbol if found; null otherwise.
 */
static PL_LITERAL* dpll_find_pure(PL_DPLL *dpll)
{
    for (uint32_t i = 0; i < PL_MAX_VARS; i++) {
        if (dpll->interpretation->dictionary[i] == PL_UNDEFINED) {
            PL_LITERAL *literal = dpll_find_pure_for_variable(dpll, (char)i);
            if (literal) {
                return literal;
            }
        }
    }
    return NULL;
}


/**
 * Searches for the next unset variable.
 *
 * @param dpll The DPLL working set.
 * @return The next unset variable.
 */
static char dpll_find_unset(PL_DPLL *dpll)
{
    for (uint32_t i = 0; i < PL_MAX_VARS; i++) {
        if (dpll->interpretation->dictionary[i] == PL_UNDEFINED) {
            return (char)i;
        }
    }
    return (char)0;
}


static uint32_t dpll_step(PL_DPLL *dpll);


/**
 * Tries to set a variable to the specified value.
 * Walks down the tree until with that variable set.
 * Returns true if the algorithm finds a satisfactory interpretation.
 * Returns false otherwise.
 * If no solution found, also uncommits the variable.
 *
 * @param dpll The DPLL working set.
 * @param variable The variable to set.
 * @param value The value to set the variable to.
 * @return True if a solution is found; false otherwise.
 */
static uint32_t dpll_try(PL_DPLL *dpll, char variable, uint8_t value)
{
    dpll->interpretation->dictionary[(uint32_t)variable] = value;

    if (dpll_step(dpll)) {
        return 1;
    }

    // Otherwise, it didn't work out.  Undo.
    dpll->interpretation->dictionary[(uint32_t)variable] = PL_UNDEFINED;
    return 0;
}


/**
 * Steps the DPLL algorithm.
 * First checks if the conjunction is satisfied.
 * Next, tries to use a "unit symbol".
 * Next, tries to use a "pure symbol".
 * Otherwise, tries to use the next unset variable.
 *
 * @param dpll The DPLL working set.
 * @return True if a solution is found; false otherwise.
 */
static uint32_t dpll_step(PL_DPLL *dpll)
{
    uint8_t result = pl_is_satisfied(dpll->conjunction, dpll->interpretation);

    if (result == PL_TRUE) {
        return 1;
    }

    if (result == PL_FALSE) {
        return 0;
    }

    PL_LITERAL *literal = dpll_find_unit(dpll);
    if (literal) {
        return dpll_try(dpll, literal->variable, literal->negated ? PL_FALSE : PL_TRUE);
    }

    literal = dpll_find_pure(dpll);
    if (literal) {
        return dpll_try(dpll, literal->variable, literal->negated ? PL_FALSE : PL_TRUE);
    }

    char variable = dpll_find_unset(dpll);
    if (dpll_try(dpll, variable, PL_TRUE)) {
        return 1;
    }

    return dpll_try(dpll, variable, PL_FALSE);
}


/**
 * Attempts to solve the conjunction using the DPLL algorithm.
 *
 * @param conjunction The conjunction.
 * @return The first interpretation that solves the conjunction.
 */
PL_INTERPRETATION* pl_dpll(PL_CONJUNCTION *conjunction)
{
    PL_DPLL dpll;
    dpll.conjunction = conjunction;
    dpll.interpretation = pl_create_interpretation(conjunction, PL_UNDEFINED);
    dpll.variables_count = 0;

    for (uint32_t i = 0; i < PL_MAX_VARS; i++) {
        if (dpll.interpretation->dictionary[i] == PL_UNDEFINED) {
            dpll.variables[dpll.variables_count++] = (char)i;
        }
    }

    if (dpll_step(&dpll)) {
        return dpll.interpretation;
    }

    // Otherwise, no solution, so clean up and return null.
    pl_print_interpretation(dpll.interpretation);
    free(dpll.interpretation);
    return NULL;
}
