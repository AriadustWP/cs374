CC      = mpicc
CFLAGS  = -DMPE_GRAPHICS -c -Wall
LFLAGS  = -o mandel -L/usr/X11R6/lib -lmpe -lX11 -lm

mandel: mandel.o display.o
	$(CC) mandel.o display.o $(LFLAGS)

mandel.o: mandel.c display.h
	$(CC) $(CFLAGS) mandel.c 

display.o: display.c display.h
	$(CC) $(CFLAGS) display.c

run1:
	mpirun -np 1 ./mandel

clean:
	rm -f mandel a.out *.o *~ *# PI*

