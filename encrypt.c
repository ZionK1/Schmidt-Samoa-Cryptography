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
                            "   Encrypts data using SS encryption.\n"
                            "   Decrypted data is encrypted by the encrypt program.\n\n"
                            "USAGE\n"
                            "   ./encrypt [OPTIONS]\n\n"
                            "OPTIONS\n"
                            "   -h              Display program help and usage.\n"
                            "   -v              Display verbose program output.\n"
                            "   -i infile       Input file of data to encrypt (default: stdin).\n"
                            "   -o outfile      Output file for encrypted data (default: stdout).\n"
                            "   -n pbfile       Public key file (default: ss.pub).\n");
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
                perror("The public key file could not be opened.");
                return 1;
            }
            break;
        case 'v': verbose = true; break;
        default:
            fprintf(stderr, "SYNOPSIS\n"
                            "   Encrypts data using SS encryption.\n"
                            "   Decrypted data is encrypted by the decrypt program.\n\n"
                            "USAGE\n"
                            "   ./encrypt [OPTIONS]\n\n"
                            "OPTIONS\n"
                            "   -h              Display program help and usage.\n"
                            "   -v              Display verbose program output.\n"
                            "   -i infile       Input file of data to encrypt (default: stdin).\n"
                            "   -o outfile      Output file for encrypted data (default: stdout).\n"
                            "   -n pbfile       Public key file (default: ss.pub).\n");
            return 1;
        }
    }

    // open public key file, printing error message in case of failure
    if (!usersetkey) {
        keyfile = fopen("ss.pub", "r");
        if (keyfile == NULL) { // in event of failure to open file
            // print error message
            perror("The public key file could not be opened.");
            return 1;
        }
    }

    // init vars used when reading in public key
    mpz_t n;
    mpz_init(n);
    char *username = getenv("USER");

    // read in public key from opened public key file
    ss_read_pub(n, username, keyfile);

    // if verbose output enabled, print respective info
    if (verbose) {
        fprintf(stdout, "user = %s\n", username);
        gmp_fprintf(stdout, "n (%d bits) = %Zd\n", mpz_sizeinbase(n, 2), n);
    }

    // encrypt file
    ss_encrypt_file(infile, outfile, n);

    // close public key file and clear mpz vars used
    fclose(infile);
    fclose(outfile);
    fclose(keyfile);
    mpz_clear(n);
    return 0;
}
