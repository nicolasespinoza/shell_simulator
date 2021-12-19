gcc *.c util/*.c -std=c99
valgrind --leak-check=full ./a.out 10 fs
