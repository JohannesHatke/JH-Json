#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "ArrayList/ArrayList.h"

#define MAX_SAFE_STR 100

// error handling

int line = 1;
void die(){
	//do some cleanup
	fprintf(stderr,"quit, error occured on line: %d\n",line);
	exit(1);
}

// hash part

/*
 * this will be used in objects where not a lot of values are expected
 */

/*
 * using dbj2 but reducing value significantly
 */
#define HASHSIZE 20

typedef struct hash_obj{
	char *name;
	void *val;
	struct hash_obj* next;
}hash_obj;

int hash(char *str){
	unsigned long hash = 5381;
	int c;
	while ((c = *str++)!='\0')
		hash = ((hash << 5)+hash) + c;

	return hash % HASHSIZE;
}

/*
 * frees every name and value associated with the hash table and the table variable itself
 */
int free_hash_table(hash_obj **table){
	int i,count;
	count = i = 0;
	hash_obj *curr,*old;
	for(i = 0; i< HASHSIZE; i++  ){
		curr = table[i];
		while(curr != NULL){
			old = curr;
			curr = curr->next;
			free(old->val);
			free(old->name);
			free(old);
			count++;
		}
	}
	free(table);
	return count;
}

hash_obj **init_hash_table(){
	hash_obj **table = malloc(HASHSIZE * sizeof(hash_obj*));
	for(int i =0; i< HASHSIZE; i++){
		*(table + i) = NULL;
	}
	return table;
}

hash_obj *lookup(hash_obj **table, char *s){
	hash_obj *curr;
	for(curr = table[hash(s)]; curr != NULL 
	&&  strncmp(s,curr->name,MAX_SAFE_STR) != 0  ; curr = curr->next)
		;
	return curr;
}

/*
 * name and val have to be malloced outside of this function
 */
hash_obj *init_hash_obj(char *name, void *val){
	hash_obj *p = malloc(sizeof(hash_obj));
	p->next = NULL;
	p->val = val;
	p->name = name;
	return p;

}

/*
 * name and val have to be malloced outside of this function
 * returns NULL on failure
 */
hash_obj *install (hash_obj **table, char *name, void *val){
	int hashval = hash(name);
	hash_obj *curr = table[hashval];
	if(curr == NULL){
		table[hashval] = init_hash_obj(name, val);
		return table[hashval];
	}

	for(curr = table[hashval]; curr->next != NULL && strncmp(name,curr->name,MAX_SAFE_STR) != 0; curr = curr->next)
		;
	if (strncmp(name,curr->name,MAX_SAFE_STR) == 0){
		fprintf(stderr, "name %s is used twice in same hash table\n",	name);
		return NULL;
	}


	curr->next = init_hash_obj(name, val);

	return curr->next;
}

enum lit_token {
	JSON_TRUE = 1000,
	JSON_FALSE,
	JSON_NULL,
};

enum val_type {
	JSON_NUM = 2000,
	JSON_CHAR,
	JSON_STR,
	JSON_OBJ,
	JSON_LITERAL,
	JSON_LIST

};


typedef struct json_val{
	void *data;
	int type;
}json_val;


typedef struct json_obj{
	char *key;
	json_val *val;

}json_obj;


/* copying the value
*
*
*/
void *derefer(json_val *to_dereference, int *intval, int *boolval){
	switch (to_dereference->type){
		case JSON_NUM:
			*intval = *((int*) to_dereference->data);
			return to_dereference->data;
		break;
		case JSON_LITERAL:
			if (*((int*) to_dereference->data) == JSON_NULL)
				*boolval = JSON_NULL;
			if (*((int*) to_dereference->data) == JSON_FALSE)
				*boolval = 0;
			if (*((int*) to_dereference->data) == JSON_TRUE)
				*boolval = 1;
		break;
	}
	return NULL;
}

/*
 * wrapper for getchar
 */
int readchar(FILE *stream){
	int c;
	while(isspace(c = fgetc(stream)))
		if(c == '\n')
			line++;
	return c;
}

int skipwhitespace(FILE *stream){
	int c,count =0;
	while (isspace(c = getc(stream))){
		if ( c== '\n')
			line++;
		count++;
	}
	if (c == EOF){
		fprintf(stderr, "encountered EOF\n");
		die();
	}
	ungetc(c,stream);
	return count;
}

/*
 * only works for Char, num, str, and literals
 */
void json_val_free(json_val *p){
	switch( p->type){
		case JSON_NUM:
			free( (int*) p->data);
		break;
		case JSON_STR:
		case JSON_CHAR:
			free( (char*) p->data);
		break;
		case JSON_LITERAL:
			free( (char*) p->data);
		break;
		default:
			fprintf(stderr,"trying to free an unsupported json value of type %d\n",p->type);
			die();
		break;
	}
	free(p);
}
void AL_free_wrapper(void *p, int pos){
	json_val_free((json_val*) p);
}


json_val *parse_Val(FILE *stream);
json_val *parse_Array(FILE *stream){
	//first character i get is always [
	int c;
	if (( c = readchar(stream)) != '['){
		fprintf(stderr,"parse_Array failed\n");
		die();
	}
	json_val *output = malloc(sizeof(json_val));
	output->type = JSON_LIST;
	output->data = AL_init(1);
	
	json_val *tmp;
	while ((c = readchar(stream))!= ']' && c != EOF){
		ungetc(c,stream);
		if ( (tmp = parse_Val(stream)) == NULL ){
			AL_foreach(output->data, &AL_free_wrapper );
			AL_free(output->data);
			free(output);
			fprintf(stderr,"failed on %d\n",c);
			die();
		}
		AL_append(output->data,(void*) tmp);

		if (( c = readchar(stream)  != ','))
			ungetc(c,stream);
	}
	return output;
}

json_val *parse_Object(FILE *stream);
json_val *parse_Array(FILE *stream);
json_val *parse_String(FILE *stream);
json_val *parse_Lit(FILE *stream);

json_val *parse_Object(FILE *stream){return NULL;}
json_val *parse_String(FILE *stream){return NULL;}
json_val *parse_Lit(FILE *stream){return NULL;}

json_val *parse_Num(FILE *stream){
	int c;
	float val;
	int sign = 1;

	if ((c = getc(stream)) == '-'){
		sign = -1;
	} else {
		ungetc(c, stream);
	}

	if ((c = getc(stream)) == '0'){
		if (isdigit((c = getc(stream)))){
			fprintf(stderr,"number cant start with leading 0\n");
			return NULL;
		}else{
			val = 0;
			ungetc(c,stream);
		}
			
	} else {
		ungetc(c, stream);
	}

	if (fscanf(stream,"%f",&val) != 1 && val != 0){
		fprintf(stderr,"error reading number\n");
		return NULL;
	}

	json_val *output = malloc(sizeof(json_val));
	float *output_data = (float*) malloc(sizeof(float));
	*output_data = sign * val;
	output->type = JSON_NUM;
	output->data = (void*) output_data;
	return output;
}

json_val *parse_Val(FILE *stream){
	int c;
	json_val *output = NULL;
	if ((c = readchar(stream)) == EOF){
		return NULL;
	}
	ungetc(c,stream);
	switch(c){
		case '{':
			return parse_Object(stream);
		break;
		case '[':
			return parse_Array(stream);
		break;
		case '\"':
			return parse_String(stream);
		break;
	}

	if ( strchr("-0123456789",c) != NULL){
		return parse_Num(stream);
	} else {
		return parse_Lit(stream);
	}
}

/*
 *  try parsing. 
 *  On failure it will cleanup all allocated memory and exit program
*/
json_val *parse_Start(FILE *stream){
	json_val *output;
	if ((output = parse_Val(stream)) != NULL){
		return output;
	}
	fprintf(stderr,"Failed parsing json\n\n");
	die();
	return NULL;
}


int main(void){

	printf("üest\n");
	hash_obj **tab1 = init_hash_table();
	char *n1 = strdup("abc");
	void *p1 = (void*) malloc(sizeof(int));
	printf("installing %s:%p\t %p\n",n1,p1,(void*) install(tab1,n1,p1));
	printf("lookup results in %p\n", lookup(tab1,n1)->val);

	char *n2 = strdup("def");
	void *p2 = (void*) malloc(sizeof(int));
	printf("installing %s:%p\t%p\n",n2,p2, (void*) install(tab1,n2,p2));
	printf("lookup results in %p\n", lookup(tab1,n2)->val);

	free_hash_table(tab1);

	//json_val *retval = parse_Num(stdin);
	FILE *testjson = fopen("1test.json","r");
	json_val *retval = parse_Start(testjson);
	ArrayList *list = (ArrayList*) retval->data;
	printf("Type: %d\n",retval->type);
	printf("list: %p\n",list);
	printf("len: %d\n",list->len);
	for (int i = 0; i< list->len; i++){
		printf("[%d] = %d\n",i,*((int*) AL_get(list,i)));
	}
	//printf("read %f\n",*((float*) retval->data));



	return 0;
}
