#include "randstate.h"
#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>

// declare state var
gmp_randstate_t state;

// Initializes the global random state with a
// Mersenne Twister algorithm, using seed as the random seed.
void randstate_init(uint64_t seed) {
    // initialize the mersenne twister algorithm and set the seed (for gmp functions)
    gmp_randinit_mt(state);
    gmp_randseed_ui(state, seed);
    // set seed for C random functions
    srandom(seed);
}

// Clears and frees all memory used by the initialized global random state
void randstate_clear(void) {
    gmp_randclear(state);
}
