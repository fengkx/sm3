all: clean sm3

sm3:
	cc -o sm3 -g3 -gdwarf-2 sm3.c
clean:
	rm -f sm3
