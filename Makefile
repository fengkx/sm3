all: clean sm3 main

main: sm3
	cc -o sm3 -O3 sm3.o main.c

sm3:
	cc -c -O3  sm3.c
clean:
	rm -f sm3 *.o
