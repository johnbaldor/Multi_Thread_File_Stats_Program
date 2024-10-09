CC = gcc
CFLAGS = -Wall
LIB = -lpthread

proj3: proj3.o
	$(CC) $(CFLAGS) -o proj3 proj3.o $(LIB)

proj3.o: proj3.c
	$(CC) $(CFLAGS) -c proj3.c

clean:
	rm -f *.o proj3