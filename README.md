# Assignment 5: Public Key Cryptography

## Description:
	In this assignment, we create three programs: keygen, encrypt, and decrypt. The keygen program
	is in charge of key generation, producing SS public and private key pairs. The encrypt program
	encrypt files using a public key and the decrypt program decrypts the encrypted files using
	the corresponding private key.
	
## Build:

To build the program:

```
$ make
```

## Running:

To run the keygen program:

```
$ ./keygen [OPTIONS]
```

```
OPTIONS:
    -h              Display program help and usage.
    -v              Display verbose program output.
    -b bits         Minimum bits needed for public key n (default: 256).
    -i iterations   Miller-Rabin iterations for testing primes (default: 50).
    -d pvfile       Private key file (default: ss.priv).
    -s seed         Random seed for testing.
```

To run the encrypt program:

```
$ ./encrypt [OPTIONS]
```

```
OPTIONS:
    -h              Display program help and usage.
    -v              Display verbose program output.
    -i infile       Input file of data to encrypt (default: stdin).
    -o outfile      Output file for encrypted data (default: stdout).
    -n pbfile       Public key file (default: ss.pub).
```

To run the decrypt program:

```
$ ./decrypt [OPTIONS]
```

```
OPTIONS:
    -h              Display program help and usage.
    -v              Display verbose program output.
    -i infile       Input file of data to decrypt (default: stdin).
    -o outfile      Output file for decrypted data (default: stdout).
    -n pvfile       Private key file (default: ss.priv).
```

## Cleaning:

To clean the program files:

```
$ make clean
```

## Formatting:

To format the program files:

```
$ make format
```

## Files:

### decrypt.c
```
This contains the implementation and main() functions for the decrypt program.
```

### encrypt.c
```
This contains the implementation and main() functions for the encrypt program.
```

### keygen.c
```
This contains the implementation and main() functions for the keygen program.
```

### numtheory.c
```
This contains the implementation of the number theory functions.
```

### numtheory.h
```
This specifies the interface for the number theory functions.
```

### randstate.c
```
This contains the implementation of the random state interface.
```

### randstate.h
```
This contains the interface for initializing and clearing the random state.
```

### ss.c
```
This contains the implementation of the SS library.
```

### ss.h
```
This specifies the interface for the SS library.
```
