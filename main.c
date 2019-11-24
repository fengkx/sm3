#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sm3.h"

#define MAXBUF 4096 
#define SM3_SIZE 32

static char buf[MAXBUF];

int main() {
	// char *s = fgets(buf, MAXBUF, stdin);
	// if(!s) {
	// 	fprintf(stderr, "input empty");
	// 	exit(1);
	// }
	// s[strlen(s) - 1] = '\0'; // remove the \n
	byte *hash = sm3(stdin);
	printf("sm3 hash: \n");
	for (int i=0;i<SM3_SIZE;i++) {
		printf("%x", hash[i]);
		if((i+1) % 4 == 0) printf(" ");
	}
	free(hash);
	printf("\n");
	return 0;
}
