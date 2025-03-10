CFLAGS = --std=c99 -Wall -Werror 
LDFLAGS = -lm
CC = gcc

showFDtables: showFDtables.c fdstruct.o processcla.o fddisplay.o getprocess.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o : %.c %.h fdstruct.h
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm *.o
