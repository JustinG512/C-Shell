CC=gcc
CFLAGS=-I.
DEPS = myshell.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

myshell: myshell.o myshellfunc.o
	$(CC) -o myshell myshell.o myshellfunc.o -Wall -Werror

clean:
	rm myshell myshell.o myshellfunc.o