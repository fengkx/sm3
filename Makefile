all: clean sm3 main

main: sm3
	cc -o sm3 -g3 -gdwarf sm3.o main.c

sm3:
	cc -c -g3 -gdwarf-2 sm3.c
clean:
	rm -f sm3
