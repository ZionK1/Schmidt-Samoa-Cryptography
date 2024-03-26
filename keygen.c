#include "ss.h"
#include "numtheory.h"
#include "randstate.h"
#include <stdio.h>
#include <gmp.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>

#define OPTIONS "hb:i:n:d:s:v"

int main(int argc, char **argv) {
    // set default values for kegen
    uint64_t iters = 50;
    uint64_t minbits = 256;
    uint64_t seed = time(NULL);
    bool verbose = false;
    bool usersetpub = false;
    bool usersetpriv = false;
    FILE *pbfile;
    FILE *pvfile;

    int opt = 0;
    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'h':
            fprintf(stderr,
                "SYNOPSIS\n"
                "   Generates an SS public/private key pair.\n\n"
                "USAGE\n"
                "   ./keygen [OPTIONS]\n\n"
                "OPTIONS\n"
                "   -h              Display program help and usage.\n"
                "   -v              Display verbose program output.\n"
                "   -b bits         Minimum bits needed for public key n (default: 256).\n"
                "   -i iterations   Miller-Rabin iterations for testing primes (default: 50).\n"
                "   -n pbfile       Public key file (default: ss.pub).\n"
                "   -d pvfile       Private key file (default: ss.priv).\n"
                "   -s seed         Random seed for testing.\n");
            return 0;
        case 'b':
            // specify minimum bits for n
            minbits = strtoul(optarg, NULL, 10);
            break;
        case 'i':
            // specify number of iterations for testing primes
            iters = strtoul(optarg, NULL, 10);
            break;
        case 'n':
            // specify public key file
            pbfile = fopen(optarg, "w");
            if (pbfile == NULL) { // in event of failure to open file
                // print error message
                perror("The public key file could not be opened.");
                return 1;
            }
            usersetpub = true;
            break;
        case 'd':
            // specify private key file
            pvfile = fopen(optarg, "w");
            if (pvfile == NULL) { // in event of failure to open file
                // print error message
                perror("The private key file could not be opened.");
                return 1;
            }
            usersetpriv = true;
            break;
        case 's':
            // specify random seed
            seed = strtoul(optarg, NULL, 10);
            break;
        case 'v':
            // enable verbose output
            verbose = true;
            break;
        default:
            fprintf(stderr,
                "SYNOPSIS\n"
                "   Generates an SS public/private key pair.\n\n"
                "USAGE\n"
                "   ./keygen [OPTIONS]\n\n"
                "OPTIONS\n"
                "   -h              Display program help and usage.\n"
                "   -v              Display verbose program output.\n"
                "   -b bits         Minimum bits needed for public key n (default: 256).\n"
                "   -i iterations   Miller-Rabin iterations for testing primes (default: 50).\n"
                "   -n pbfile       Public key file (default: ss.pub).\n"
                "   -d pvfile       Private key file (default: ss.priv).\n"
                "   -s seed         Random seed for testing.\n");
            return 1;
        }
    }
    // if pbfile not set/opened by user
    if (!usersetpub) {
        // open default file
        pbfile = fopen("ss.pub", "w");
        if (pbfile == NULL) { // in event of failure to open file
            // print error message
            perror("The public key file could not be opened.");
            return 1;
        }
    }
    // if pvfile not set/opened by user
    if (!usersetpriv) {
        // open default file
        pvfile = fopen("ss.priv", "w");
        if (pvfile == NULL) { // in event of failure to open file
            // print error message
            perror("The private key file could not be opened.");
            return 1;
        }
    }
    // set private key file permissions
    fchmod(fileno(pbfile), 0600);

    // initialize random state
    randstate_init(seed);

    // make public and private keys
    mpz_t p, q, n, d, pq;
    mpz_inits(p, q, n, d, pq, NULL);
    ss_make_pub(p, q, n, minbits, iters);
    ss_make_priv(d, pq, p, q);

    // get username
    char *username = getenv("USER");

    // write public and private key to respective files
    ss_write_pub(n, username, pbfile);
    ss_write_priv(pq, d, pvfile);

    // if verbose output enabled, print values used for encryption
    if (verbose) {
        fprintf(stderr, "user = %s\n", username);
        gmp_fprintf(stderr, "p (%i bits) = %Zd\n", mpz_sizeinbase(p, 2), p);
        gmp_fprintf(stderr, "q (%i bits) = %Zd\n", mpz_sizeinbase(q, 2), q);
        gmp_fprintf(stderr, "n (%i bits) = %Zd\n", mpz_sizeinbase(n, 2), n);
        gmp_fprintf(stderr, "pq (%i bits) = %Zd\n", mpz_sizeinbase(pq, 2), pq);
        gmp_fprintf(stderr, "d (%i bits) = %Zd\n", mpz_sizeinbase(d, 2), d);
    }

    // close files, clear random state, clear mpz_t variables used
    fclose(pbfile);
    fclose(pvfile);
    randstate_clear();
    mpz_clears(p, q, n, d, pq, NULL);
    return 0;
}
