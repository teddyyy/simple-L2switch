PROGS = switch

all: $(PROGS)

switch: switch.o netutil.o hash.o
	$(CC) switch.o netutil.o hash.o -o switch
	rm *.o

switch.o: switch.c
	$(CC) -c switch.c

clean:
	rm switch
