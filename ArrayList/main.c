#include <alloca.h>
#include <stdio.h>
#include <stdlib.h>
#include "ArrayList.h"

void free_entry(void *p, int pos){
	free(p);
}
int main(){
	ArrayList *test = AL_init(1);
	for (int i = 0; i< 100;i++) {
		int *a = malloc(sizeof(int));
		AL_append(test,(void*) a);
	}
	for (int i = 0; i< 100;i++) {
		printf("%4d <=> %p\n",i,( AL_get(test,i)));
	}
	AL_foreach(test, free_entry);
	AL_free(test);
}
