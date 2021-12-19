CC = gcc
CFLAGS = -Wall -pedantic -g -std=c99
MAIN = main
OBJS = main.o toolkit.o array_list.o emulator_info.o

all : $(MAIN)

$(MAIN) : $(OBJS) util/toolkit.h util/array_list.h emulator_info.h
	$(CC) $(CFLAGS) -o $(MAIN) $(OBJS)

main.o : main.c util/toolkit.h util/array_list.h emulator_info.h
	$(CC) $(CFLAGS) -c main.c

toolkit.o : util/toolkit.c
	$(CC) $(CFLAGS) -c util/toolkit.c

array_list.o : util/array_list.c
	$(CC) $(CFLAGS) -c util/array_list.c

emulator_info.o : emulator_info.c
	$(CC) $(CFLAGS) -c emulator_info.c

clean :
	rm *.o $(MAIN)