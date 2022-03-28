PROG = circuitSatisfiability
CC = mpicc
CFLAGS = -Wall -ansi -pedantic -std=c99 # -ansi : follow the standard || -pedantic : strict implementation of ansi
LFLAGS = -o $(PROG)

$(PROG): $(PROG).c
	$(CC) $(CFLAGS) $(PROG).c $(LFLAGS)

clean:
	rm -f $(PROG) a.out *~ *# *.o

