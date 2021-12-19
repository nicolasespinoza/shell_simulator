gcc *.c util/*.c -g -std=c99
valgrind --leak-check=full ./a.out 10 fs
