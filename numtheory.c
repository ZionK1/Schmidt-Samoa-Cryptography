#include "numtheory.h"
#include "randstate.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <gmp.h>

// Performs modular exponentiation, computing the base (a) raised to the
// exponent power (d) modulo modulus (n) and storing the result in output (o)
void pow_mod(mpz_t o, mpz_t a, mpz_t d, mpz_t n) {
    mpz_t p, v, dd;
    mpz_init_set(dd, d);
    // v = 1
    mpz_init_set_ui(v, 1);
    // p = a
    mpz_init_set(p, a);
    // while d > 0
    while (mpz_cmp_ui(dd, 0) > 0) {
        // if d % 2 == 1
        if (mpz_odd_p(dd) != 0) {
            // o = (v * p) % n
            mpz_mul(v, v, p); // (v * p)
            mpz_mod(v, v, n); // (v * p) % n
        }
        // p = (p * p) % n
        mpz_mul(p, p, p); // (p * p)
        mpz_mod(p, p, n); // (p * p) % n
        // d /= 2
        mpz_fdiv_q_ui(dd, dd, 2);
    }
    mpz_set(o, v);
    mpz_clears(v, p, dd, NULL);
}

// Conducts the Miller-Rabin primality test to indicate whether or not n is prime
bool is_prime(mpz_t n, uint64_t iters) {
    // some obvious cases to check for before doing too much:
    // if n is even and greater than 2, not prime
    if (mpz_get_ui(n) % 2 == 0 && mpz_cmp_ui(n, 2) > 0) {
        return false;
    }
    // if n = 0 or 1, not prime
    else if (mpz_cmp_ui(n, 0) == 0 || mpz_cmp_ui(n, 1) == 0) {
        return false;
    }
    // if n = 2 or 3, is prime
    else if (mpz_cmp_ui(n, 3) == 0 || mpz_cmp_ui(n, 2) == 0) {
        return true;
    }
    // write n - 1 = 2^s*r such that r is odd
    // r = n - 1
    // while (is_even(r))
    // r //= 2
    // s += 1
    mpz_t r, s, i, k, a, y, nmin1, nmin3, j, smin1, two;
    mpz_inits(r, s, i, k, a, y, nmin1, nmin3, j, smin1, two, NULL);
    mpz_sub_ui(r, n, 1);
    while (mpz_even_p(r) != 0) {
        mpz_fdiv_q_ui(r, r, 2);
        mpz_add_ui(s, s, 1);
    }
    mpz_set_ui(k, iters);
    // for (i = 1; i < k; i++)
    for (mpz_init_set_ui(i, 1); mpz_cmp(i, k) < 0; mpz_add_ui(i, i, 1)) {
        // choose random a = {2, 3, ..., n - 2}
        mpz_sub_ui(nmin3, n, 3);
        mpz_urandomm(a, state, nmin3); // a = {0, n - 4}
        mpz_add_ui(a, a, 2); // a = {2, n - 2}
        // y = pow_mod(a, r, n);
        pow_mod(y, a, r, n);
        mpz_sub_ui(nmin1, n, 1);
        // if (y != 1 && y != n - 1)
        if (mpz_cmp_ui(y, 1) != 0 && mpz_cmp(y, nmin1) != 0) {
            // j = 1
            mpz_set_ui(j, 1);
            mpz_sub_ui(smin1, s, 1);
            // while (j <= s - 1 && y != n - 1)
            while (mpz_cmp(j, smin1) <= 0 && mpz_cmp(y, nmin1) != 0) {
                // y = pow_mod(y, 2, n)

                mpz_set_ui(two, 2);
                pow_mod(y, y, two, n);
                // if (y == 1)
                if (mpz_cmp_ui(y, 1) == 0) {
                    mpz_clears(r, s, i, k, a, y, nmin1, nmin3, j, smin1, two, NULL);
                    return false;
                }
                // j += 1
                mpz_add_ui(j, j, 1);
            }
            // if (y != n - 1)
            if (mpz_cmp(y, nmin1) != 0) {
                mpz_clears(r, s, i, k, a, y, nmin1, nmin3, j, smin1, two, NULL);
                return false;
            }
        }
    }
    mpz_clears(r, s, i, k, a, y, nmin1, nmin3, j, smin1, two, NULL);
    return true;
}

// Generates a new prime number that is at least bits number of bits long.
void make_prime(mpz_t p, uint64_t bits, uint64_t iters) {
    bool prime_made = false;
    // until prime is made
    while (!prime_made) {
        // generate random number as mpz var
        mpz_urandomb(p, state, bits);
        // check if random number is prime and is at least size of bits
        if (is_prime(p, iters) && mpz_sizeinbase(p, 2) + 1 >= bits) {
            prime_made = true;
        }
    }
    return;
}

// Computes the greatest common divisor of a and b
void gcd(mpz_t g, mpz_t a, mpz_t b) {
    mpz_t adup, bdup, tmp;
    mpz_inits(adup, bdup, tmp, NULL);
    mpz_set(bdup, b);
    mpz_set(adup, a);
    // while b != 0
    while (mpz_cmp_ui(bdup, 0) != 0) {
        // t = b
        mpz_set(tmp, bdup);
        // b = a mod b
        mpz_mod(bdup, adup, bdup);
        // a = t
        mpz_set(adup, tmp);
    }
    mpz_set(g, adup);
    mpz_clears(adup, bdup, tmp, NULL);
}

// Computes the inverse i of a modulo n.
// In the case mod inverse can't be found, o = 0.
void mod_inverse(mpz_t o, mpz_t a, mpz_t n) {
    mpz_t r, rp, t, tp, q, tmp;
    mpz_inits(r, rp, t, tp, q, tmp, NULL);
    // (r, r') = (n, a)
    mpz_set(r, n);
    mpz_set(rp, a);
    // (t, t') = (0, 1)
    mpz_set_ui(t, 0);
    mpz_set_ui(tp, 1);
    // while (r' != 0)
    while (mpz_cmp_ui(rp, 0) != 0) {
        // q = floordiv(r/r')
        mpz_fdiv_q(q, r, rp);
        // (r, r') = (r', r - q x r')
        mpz_set(tmp, r);
        mpz_set(r, rp); // r = rp
        mpz_mul(rp, q, rp); // rp = q x r'
        mpz_sub(rp, tmp, rp); // rp = r - q x r'
        // (t, t') = (t', t - q x t')
        mpz_init_set(tmp, t);
        mpz_set(t, tp); // t = tp
        mpz_mul(tp, q, tp); // tp = q x t'
        mpz_sub(tp, tmp, tp); // tp = t - q x t'
    }
    // if r > 1
    if (mpz_cmp_ui(r, 1) > 0) {
        mpz_set_ui(t, 0);
    }
    // if t < 0
    if (mpz_cmp_ui(t, 0) < 0) {
        mpz_add(t, t, n);
    }
    // return t
    mpz_set(o, t);
    mpz_clears(r, rp, t, tp, q, tmp, NULL);
}
