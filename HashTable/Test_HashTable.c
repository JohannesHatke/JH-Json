#include "HashTable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int print_entry(void *entry){
	printf("entry:\t%d\n",*( (int*) entry));
	return 0;
}

typedef struct json_val{
	void *data;
	int type;
}json_val;

typedef struct json_obj{
	char *key;
	json_val *val;
}json_obj;
#define JSON_HASH_SIZE 30

int json_hash_func(void *a){
	if (a == NULL){
		fprintf(stderr,"hash obj Error, NULL encountered\n");
		return -1;
	}
	json_obj *ja = (json_obj*) a;

	return ((ja->key)[0] % JSON_HASH_SIZE) ;

}




int json_obj_comp(void *a, void *b){
	if ( a == NULL || b == NULL){
		fprintf(stderr,"comparison Error, NULL encountered\n");
		return 0;
	}

	json_obj *ja = (json_obj*) a;
	json_obj *jb = (json_obj*) b;

	return !(strncmp(ja->key, jb->key, 10000));
}

void json_val_free(json_val *p){
	if(p == NULL){
		return;
	}
}

int free_json_obj(void *a){
	if (a == NULL){
		fprintf(stderr,"free obj Error, NULL encountered\n");
		return 1;
	}
	json_obj *ja = (json_obj*) a;
	free(ja->key);
	json_val_free(ja->val);
	free(a);
	return 0;
}




#define JSON_HASHTABLE_INIT() HashTable_init(JSON_HASH_SIZE, &json_hash_func,&free_json_obj, &json_obj_comp)

int print_json_obj(void *a){
	json_obj *ja = a;
	printf("%s : ?\n",ja->key);
	return 0;
}


void json_test(){

	HashTable *test =JSON_HASHTABLE_INIT();
	json_obj *ex = malloc(sizeof(json_obj));
	ex->key = malloc(4*sizeof(char));
	strncpy(ex->key,"hey",4);
	ex->val = NULL;
	Hash_install(test,ex);

	json_obj *exb = malloc(sizeof(json_obj));
	exb->key = malloc(4*sizeof(char));
	strncpy(exb->key,"he",4);
	exb->val = NULL;
	Hash_install(test,exb);

	Hash_foreach(test, &print_json_obj);
	HashTable_free(test);
}



int main(int argc, char *argv[])
{
	json_test();
	//HashTable *test = HashTable_init(HASH_FUNC_EXAMPLE_SIZE, HASH_FUNC_EXAMPLE,&Hash_free_int, &Hash_comp_int);
	HashTable *test = HashTable_int_init();

	const int numtest[] = {0,4,10,1004,2004,1 };
	printf("[");
	for(int i = 0; i < (sizeof(numtest) / sizeof(int)); i++){
		printf("%d,",numtest[i]);
		void *a = malloc(sizeof(int));
		*((int* )a) = numtest[i];
		Hash_install(test,a);
	}
	printf("]\n");
	Hash_foreach(test, &print_entry);
	int b = 4;
	int *pb = &b;
	int *c = Hash_get(test, (void*) pb);
	printf("tried to find 4 got %p:%d\n", c, (c == NULL) ? 0 :  *c);

	
	HashTable_free(test);

	return 0;
}
