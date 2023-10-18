#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_SAFE_STR 100

// error handling

int line;
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
		case JSON_LIT:
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

typedef struct node{
	json_val *val;
	struct node *next;
}node;


// TODO write a working list too tired, with append and all or just use the one i already wrote
json_val *parse_Val(FILE *stream);
json_val *parse_Array(FILE *stream){
	//first character i get is always [
	int c;
	node *curr,*old;
	if (( c = fgetc(stream)) != '['){
		fprintf(stderr,"parse_Array failed\n");
	}
	json_val *output = malloc(sizeof(json_val));
	output->type = JSON_LIST;
	output->data = NULL;
	skipwhitespace(stream);
	if ((c = fgetc(stream))== ']')
		return output;

	ungetc(c, stream);
	output->data = malloc(sizeof(node));
	old = (node*) output->data;
	while ((c = fgetc(stream))!= ']'){
		old = curr;
		curr->next = NULL;
		curr->val = parse_Val(stream);
	}



}

json_val *parse_Object(FILE *stream);
json_val *parse_Array(FILE *stream);
json_val *parse_String(FILE *stream);
json_val *parse_Num(FILE *stream);
json_val *parse_Lit(FILE *stream);

json_val *parse_Val(FILE *stream){
	int c;
	if ((c = fgetc(stream)) == EOF){
		die();
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

	if ( strchr("-123456789",c) != NULL){
		return parse_Num(stream);
	} else {
		return parse_Lit(stream);
	}

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



	return 0;
}
