#include "ss.h"
#include "numtheory.h"
#include "randstate.h"
#include <stdio.h>
#include <gmp.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

#define OPTIONS "hi:o:n:v"

int main(int argc, char **argv) {
    // set defaults for encrypt
    bool usersetkey = false;
    bool verbose = false;
    FILE *infile = stdin;
    FILE *outfile = stdout;
    FILE *keyfile;

    int opt = 0;
    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'h':
            fprintf(stderr, "SYNOPSIS\n"
                            "   Decrypts data using SS decryption.\n"
                            "   Encrypted data is decrypted by the decrypt program.\n\n"
                            "USAGE\n"
                            "   ./decrypt [OPTIONS]\n\n"
                            "OPTIONS\n"
                            "   -h              Display program help and usage.\n"
                            "   -v              Display verbose program output.\n"
                            "   -i infile       Input file of data to decrypt (default: stdin).\n"
                            "   -o outfile      Output file for decrypted data (default: stdout).\n"
                            "   -n pvfile       Private key file (default: ss.priv).\n");
            return 0;
        case 'i':
            infile = fopen(optarg, "r");
            if (infile == NULL) { // in event of failure to open file
                // print error message
                perror("The input file could not be opened.");
                return 1;
            }
            break;
        case 'o':
            outfile = fopen(optarg, "w+");
            if (outfile == NULL) { // in event of failure to open file
                // print error message
                perror("The output file could not be opened.");
                return 1;
            }
            break;
        case 'n':
            usersetkey = true;
            keyfile = fopen(optarg, "r");
            if (keyfile == NULL) { // in event of failure to open file
                // print error message
                perror("The private key file could not be opened.");
                return 1;
            }
            break;
        case 'v': verbose = true; break;
        default:
            fprintf(stderr, "SYNOPSIS\n"
                            "   Decrypts data using SS decryption.\n"
                            "   Encrypted data is decrypted by the decrypt program.\n\n"
                            "USAGE\n"
                            "   ./decrypt [OPTIONS]\n\n"
                            "OPTIONS\n"
                            "   -h              Display program help and usage.\n"
                            "   -v              Display verbose program output.\n"
                            "   -i infile       Input file of data to decrypt (default: stdin).\n"
                            "   -o outfile      Output file for decrypted data (default: stdout).\n"
                            "   -n pvfile       Private key file (default: ss.priv).\n");
            return 1;
        }
    }

    // if user hasn't set key file, open private key file
    // printing error message in case of failure
    if (!usersetkey) {
        keyfile = fopen("ss.priv", "r");
        if (keyfile == NULL) { // in event of failure to open file
            // print error message
            perror("The private key file could not be opened.");
            return 1;
        }
    }

    // init vars used when reading in private key
    mpz_t pq, d;
    mpz_inits(pq, d, NULL);

    // read in private key from opened private key file
    ss_read_priv(pq, d, keyfile);

    // if verbose output enabled, print respective info
    if (verbose) {
        gmp_fprintf(stdout, "pq (%d bits) = %Zd\n", mpz_sizeinbase(pq, 2), pq);
        gmp_fprintf(stdout, "d (%d bits) = %Zd\n", mpz_sizeinbase(d, 2), d);
    }

    // decrypt file
    ss_decrypt_file(infile, outfile, d, pq);

    // close private key file and clear mpz vars used
    fclose(infile);
    fclose(outfile);
    fclose(keyfile);
    mpz_clears(pq, d, NULL);
    return 0;
}
