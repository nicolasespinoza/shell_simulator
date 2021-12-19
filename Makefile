CC = gcc
CFLAGS = -Wall -pedantic -g -std=c99
MAIN = main
OBJS = main.o toolkit.o array_list.o emulator.o virtual_file_system.o

all : $(MAIN)

$(MAIN) : $(OBJS) util/toolkit.h util/array_list.h emulator_info.h virtual_file_system.h
	$(CC) $(CFLAGS) -o $(MAIN) $(OBJS)

main.o : main.c util/toolkit.h util/array_list.h emulator_info.h virtual_file_system.h
	$(CC) $(CFLAGS) -c main.c

toolkit.o : util/toolkit.c
	$(CC) $(CFLAGS) -c util/toolkit.c

array_list.o : util/array_list.c
	$(CC) $(CFLAGS) -c util/array_list.c

emulator.o : emulator_info.c
	$(CC) $(CFLAGS) -c emulator.c

virtual_file_system.o : virtual_file_system.c
	$(CC) $(CFLAGS) -c virtual_file_system.c

clean :
	rm *.o $(MAIN) core*