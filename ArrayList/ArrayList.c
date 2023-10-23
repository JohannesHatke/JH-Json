#include <stdio.h>
#include <stdlib.h>


/*
 maximum Size (10^30) * startingSize
 */
typedef struct ArrayList{
	int len;
	int startSize;
	void ***entries;  /*
	2d array of void pointers
	has size [100][n_i] where n_i depends on the first index
	according to the following formula
		n_i = 2^i * startSize
	
	so 
	  [0][1 * startsize]
	  [1][2 * startsize]
	and so on	
	*/

}ArrayList;

/*
#define AL_get(AL,i) AL->entries[i ];
#define AL_set(AL,i,val) \
	free(AL->entries[i]); \
	AL->entries[i] = val;
*/


int ownPow(int base, int power){ // avoiding math.h
	if (power < 0)
		return 0;

	int output = 1;
	while (power--)
		output *= base;
	return output;

}
ArrayList *AL_init(int size){
	ArrayList *output = malloc(sizeof(ArrayList));
	output->len = 0;
	output->startSize = size;
	output->entries = (void*) calloc(100 , sizeof(void*));
	return output;
}

void getIndex(ArrayList *al,int pos, int *index1, int *index2){
	int passed = 0;
	for (*index1 = 0; *index1< 100; (*index1)++){
		if ( pos - passed < al->startSize * ownPow(2, *index1))
			break;
		passed += al->startSize * ownPow(2, *index1);

	}
	*index2 = pos - passed;
}

void AL_set(ArrayList *al,int pos, void *val){
	al->len = (pos >= al->len) ? pos + 1 : al->len;
	int index1,index2;
	getIndex(al, pos,&index1,&index2);

	if (al->entries[index1] == NULL)
		al->entries[index1] = (void*) calloc(ownPow(2, index1), sizeof(void*)* al->startSize );

	al->entries[index1][index2] = val;
}
void *AL_get(ArrayList *al,int pos){
	int index1,index2;

	getIndex(al, pos, &index1, &index2);
	if (al->entries[index1] == NULL)
		return NULL;

	return al->entries[index1][index2];
}

/*
* execute the given function for each filled entry.
* The parameters it gets are the void pointer to the entry and a position
*/
int AL_foreach(ArrayList *al, void (*fp)(void*, int)){
	int pos,index1,index2, func_calls;
	pos = index1 = index2= func_calls =0;
	while ( pos < al->len){
		//moving from one index to another
		//either when at the end or when it hasn't been filled.
		if(index2 == ownPow(2, index1) * al->startSize 
			|| al->entries[index1] == NULL){
			index1++;
			index2 = 0;
			continue;
		}

		if (al->entries[index1][index2] != NULL){
			func_calls++;
			fp(al->entries[index1][index2],pos);
		}
		pos++;
		index2++;
	}
	return func_calls;
}

void AL_append(ArrayList *al,void *val){
	AL_set(al,al->len,val);
}
void AL_free(ArrayList *al){
	for (int i = 0; i< 100; i++){
		if(al->entries[i] != NULL)
			free(al->entries[i]);
	}
	free(al->entries);
	free(al);
}



