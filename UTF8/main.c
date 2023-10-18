#include "utf8.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv){
	char *test = strdup("↖");
	printf("%p\n",test);
	printf( "ü:%ld\n", utf8_decode(test));
	printf("\n\n");

	int toencode = 220;
	printf("%d: %s\n",toencode,utf8_encode(toencode));

	char *s;
	for(int i = 8352; i< 10000; i++){
		s = utf8_encode(i);
		printf("U+%05x:%-10s%ld\n",i,s,utf8_decode(s));
		free(s);
	}
	return 0;
}
