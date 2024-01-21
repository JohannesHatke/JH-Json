#include <stdlib.h>
#include "HashTable.h"


/* int section */

int example_hash(void *val){
	return *( (int*) val) % HASH_FUNC_EXAMPLE_SIZE;
}

int Hash_free_int(void *entry){
	free( (int*) entry);
	return 0;
}

int Hash_comp_int(void *a, void *b){
	if ( *((int*) a) == *((int*) b))
		return 1;
	return 0;

}


/* generic funcitons */

HashTable *HashTable_init(int size, 
			  int (*hash_func) (void *), int (*free_func) (void *),
			  int (*comp_func) (void *,void *)){
	HashTable  *output;
	if((output = malloc(sizeof(HashTable))) != NULL){
		output->size = size;
		output->hash = hash_func;
		output->comp_func = comp_func;
		output->free_func = free_func;
		output->entries = calloc(size,sizeof(Hash_node*));
	}
	return output;
}



void HashTable_free(HashTable  *table){
	int count = 0;
	Hash_node *p;
	Hash_node *curr,*old;
	for(int i = 0; i< table->size; i++){
		p = table->entries[i];
		if (p != NULL ){
			curr = p->next;
			(*(table->free_func))(p->val);
			free(p);
			while(curr){
				old = curr;
				curr = curr->next;
				(*(table->free_func))(old->val);
				free(old);
				count++;
			}
		}
		
	}

	free(table->entries);
	free(table);
}

int Hash_foreach(HashTable *table, int (*func) (void*,int)){
	int count = 0;
	Hash_node *p;
	Hash_node *curr;
	for(int i = 0; i< table->size; i++){
		p = table->entries[i];
		if (p != NULL && p->val != NULL){
			count++;
			func(p->val,count-1);
			curr = p->next;
			while(curr){
				count++;
				func(curr->val,count-1);
				curr = curr->next;
			}
		}
		
	}

	return count;
}

int Hash_install(HashTable *table, void *value){
	int pos;
	Hash_node *new;
	if (value ==NULL)
		return 1;

	pos = (*(table->hash)) (value); //make sure function returns in [0, size]
	if (pos < 0 || pos > table->size)
		return 2;

	new = malloc(sizeof(Hash_node));
	new->val = value;

	new->next = table->entries[pos];
	table->entries[pos] = new;
	return 0;
}


/*
* removes the first (!) occurence of the value to be found
* table: 
* value: the value to be found
* comp: function that should return 1 if values are the same, and 0 if not
* returns pointer to the value if it was found
* returns Null if value was not found or error occurs
*/
void *Hash_remove_get(HashTable *table,void *value){
	int pos;
	Hash_node *curr,*found;
	void *output = NULL;
	int (*comp)(void*, void*) = table->comp_func;


	if (value ==NULL)
		return NULL;
	pos = (*(table->hash)) (value); //make sure function returns in [0, size]
	if (pos < 0 || pos > table->size)
		return NULL;

	curr = table->entries[pos];
	if((*comp)(value, curr->val)){ // first entry at pos
		found = curr;
		output = found->val;
		table->entries[pos] = found->next;

		free(found);
		return output;
	} else {
		for (; curr->next != NULL; 
		curr = curr->next){
			if( (*comp)(value, (curr->next)->val)){
				found = curr->next;
				curr->next = found->next;
				output = found->val;
				free(found);
				return output;
			};
		}
	}
	return output;
}


/*
 * returns a pointer to the value that is the same 
 * as the parameter according to the comp function
 */
void *Hash_get(HashTable *table,void *value){
	int pos;
	Hash_node *curr,*found;
	void *output = NULL;
	int (*comp)(void*, void*) = table->comp_func;


	if (value ==NULL)
		return NULL;
	pos = (*(table->hash)) (value); //make sure function returns in [0, size]
	if (pos < 0 || pos > table->size)
		return NULL;

	curr = table->entries[pos];
	if((*comp)(value, curr->val)){ // first entry at pos
		found = curr;
		output = found->val;
		return output;
	} else {
		for (; curr->next != NULL; 
		curr = curr->next){
			if( (*comp)(value, (curr->next)->val)){
				found = curr->next;
				output = found->val;
				return output;
			};
		}
	}
	return NULL;

}

int Hash_remove(HashTable *table,void *value){
	void *found = Hash_remove_get(table, value);
	if (found != NULL){
		(*(table->free_func)) (found);
		return 1;
	}
	return 0;

}



