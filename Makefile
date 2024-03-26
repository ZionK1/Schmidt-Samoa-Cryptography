CC       = clang
CFLAGS   = -Wall -Wextra -Werror -Wpedantic $(shell pkg-config --cflags gmp) -gdwarf-4
LFLAGS   = $(shell pkg-config --libs gmp)

all: keygen encrypt decrypt

keygen: keygen.o randstate.o numtheory.o ss.o
	$(CC) -o $@ $^ $(LFLAGS)

encrypt: encrypt.o randstate.o numtheory.o ss.o
	$(CC) -o $@ $^ $(LFLAGS)

decrypt: decrypt.o randstate.o numtheory.o ss.o
	$(CC) -o $@ $^ $(LFLAGS)

%.o:%.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f keygen encrypt decrypt *.o

format:
	clang-format -i -style=file *.[ch]

