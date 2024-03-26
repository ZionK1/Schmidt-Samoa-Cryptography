#include "ss.h"
#include "numtheory.h"
#include "randstate.h"
#include <stdio.h>
#include <gmp.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

// Creates parts of a new SS public key: two large primes p and q, and
// n computed as p * p * q
void ss_make_pub(mpz_t p, mpz_t q, mpz_t n, uint64_t nbits, uint64_t iters) {
    // find size of p in range [nbits/5, (2 x nbits)/5]
    uint64_t sizeofp = random() % (((2 * nbits) / 5) - (nbits / 5)) + (nbits / 5);
    // Since n = p * p * q, q = n - (2p)
    uint64_t sizeofq = nbits - (2 * sizeofp);
    // now that we have size of p and q, make the primes
    make_prime(p, sizeofp + 1, iters);
    make_prime(q, sizeofq + 1, iters);
    mpz_t psqr;
    mpz_init(psqr);
    mpz_mul(psqr, p, p); // psqr = p * p
    mpz_mul(n, psqr, q); // n = psqr * q
    mpz_clear(psqr);
}

// Writes a public SS key and username to pbfile
void ss_write_pub(mpz_t n, char username[], FILE *pbfile) {
    // print n as hexstring and username as string in pbfile, each followed by trailing newline
    gmp_fprintf(pbfile, "%Zx\n%s\n", n, username);
}

// Reads a public SS key and username from pbfile
void ss_read_pub(mpz_t n, char username[], FILE *pbfile) {
    // scan n and username in pbfile
    gmp_fscanf(pbfile, "%Zx\n%s\n", n, username);
}

// Creates a new SS private key d given primes p and q and the public key n
void ss_make_priv(mpz_t d, mpz_t pq, mpz_t p, mpz_t q) {
    // to compute d, compute the inverse of n modulo lambda(pq)
    mpz_t lampq, pmin1, qmin1, phi_n, g, n;
    mpz_inits(lampq, pmin1, qmin1, phi_n, g, n, NULL);
    mpz_sub_ui(pmin1, p, 1); // p - 1
    mpz_sub_ui(qmin1, q, 1); // q - 1
    mpz_mul(phi_n, pmin1, qmin1); // phi_n = (p-1)(q-1)
    gcd(g, pmin1, qmin1); // g = gcd(p-1, q-1)
    mpz_fdiv_q(lampq, phi_n, g); // lambda(pq) = phi_n / gcd(p-1, q-1)
    mpz_mul(n, p, p); // n = p^2
    mpz_mul(n, n, q); // n = p^2 * q
    mod_inverse(d, n, lampq); // d = inverse of n mod lambda(pq)
    mpz_mul(pq, p, q);
    mpz_clears(lampq, pmin1, qmin1, phi_n, g, n, NULL);
}

// Writes a private SS key to pvfile
void ss_write_priv(mpz_t pq, mpz_t d, FILE *pvfile) {
    // print pq then d as hexstrings, each followed by trailing newlines
    gmp_fprintf(pvfile, "%Zx\n%Zx\n", pq, d);
}

// Reads a private SS key to pvfile
void ss_read_priv(mpz_t pq, mpz_t d, FILE *pvfile) {
    // read pq then d as hexstrings, each followed by trailing newlines
    gmp_fscanf(pvfile, "%Zx\n%Zx\n", pq, d);
}

// Performs SS encryption, computing ciphertext by encrypting message
void ss_encrypt(mpz_t c, mpz_t m, mpz_t n) {
    // E(m) = c = m^n (mod n)
    pow_mod(c, m, n, n);
}

// Encrypts the contents of infile
void ss_encrypt_file(FILE *infile, FILE *outfile, mpz_t n) {
    // calculate the block size k
    mpz_t m, c;
    mpz_inits(m, c, NULL);
    uint64_t size = (0.5 * mpz_sizeinbase(n, 2) - 1) / 8; // note: log_2(sqrt(n)) = 0.5 * log_2(n)
    // dynamically allocate an array that can hold k bytes
    uint8_t *block = (uint8_t *) calloc(size, sizeof(uint8_t));
    // set zeroth byte of block to 0xFF
    block[0] = 0xFF;
    // when infile is stdin
    if (infile == stdin) {
        uint64_t j = fread(block + 1, sizeof(uint8_t), size - 1, infile);
        // convert read bytes, including prepended 0xFF into mpz_t m
        mpz_import(m, j + 1, 1, 1, 1, 0, block);
        // encrypt m
        ss_encrypt(c, m, n);
        gmp_fprintf(outfile, "%Zx\n", c);
        free(block);
        block = NULL;
        mpz_clears(m, c, NULL);
        return;
    }
    // while there are still unprocessed bytes in infile
    uint64_t bytestoread = 0; // var for total bytes to read in file
    fseek(infile, 0, SEEK_END); // set pointer in file to end of file
    bytestoread = ftell(infile); // ftell gives total size of file
    fseek(infile, 0, SEEK_SET); // set pointer in file back to top
    while (bytestoread > 0) {
        // read at most k - 1 bytes and let j be the number of bytes actually read
        uint64_t j = fread(block + 1, sizeof(uint8_t), size - 1, infile);
        if (j < 1) { // when we reach EOF
            j = fread(
                block + 1, sizeof(uint8_t), bytestoread, infile); // read in bytes left in file
        }
        bytestoread -= j; // update bytes left to read by subtracting bytes read
        // convert read bytes, including prepended 0xFF into mpz_t m
        mpz_import(m, j + 1, 1, 1, 1, 0, block);
        // encrypt m
        ss_encrypt(c, m, n);
        gmp_fprintf(outfile, "%Zx\n", c);
    }
    free(block);
    block = NULL;
    mpz_clears(m, c, NULL);
}

// Performs SS decryption, computing message by decrypting ciphertext
void ss_decrypt(mpz_t m, mpz_t c, mpz_t d, mpz_t pq) {
    // D(c) = m = c^d (mod pq)
    pow_mod(m, c, d, pq);
}

// Decrypt the contents of infile to outfile
void ss_decrypt_file(FILE *infile, FILE *outfile, mpz_t d, mpz_t pq) {
    // calculate size of block
    mpz_t c, m;
    mpz_inits(c, m, NULL);
    uint64_t size = (mpz_sizeinbase(pq, 2) - (size_t) 1) / 8;
    // dynamically allocate an array that can hold size bytes
    uint8_t *block = (uint8_t *) calloc(size, sizeof(uint8_t));
    // if infile is stdin
    if (infile == stdin) {
        // scan in ciphertext c
        gmp_fscanf(infile, "%Zx\n", c);
        // decrypt c back into m
        ss_decrypt(m, c, d, pq);
        // convert m back into bytes, stored in block
        size_t j = 0; // used as count for bytes converted
        mpz_export(block, &j, 1, 1, 1, 0, m);
        // write out j - 1 bytes starting from index 1 of the block to outfile
        fwrite(block + 1, sizeof(uint8_t), j - 1, outfile);
        free(block);
        block = NULL;
        mpz_clears(c, m, NULL);
        return;
    }
    // iterating over the lines in infile
    while (1) {
        // scan in ciphertext c
        int scan = gmp_fscanf(infile, "%Zx\n", c);
        if (scan < 1) { // if EOF is reached
            break;
        }
        // decrypt c back into m
        ss_decrypt(m, c, d, pq);
        // convert m back into bytes, stored in block
        size_t j = 0; // used as count for bytes converted
        mpz_export(block, &j, 1, 1, 1, 0, m);
        // write out j - 1 bytes starting from index 1 of the block to outfile
        fwrite(block + 1, sizeof(uint8_t), j - 1, outfile);
    }
    free(block);
    block = NULL;
    mpz_clears(c, m, NULL);
}
