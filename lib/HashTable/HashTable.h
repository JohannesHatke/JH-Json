#ifndef HASHTABLE_H
#define HASHTABLE_H

#ifdef __cplusplus 
extern "C" {
#endif


#define HASH_FUNC_EXAMPLE &example_hash
#define HASH_FUNC_EXAMPLE_SIZE 1000

int example_hash(void *val);



typedef struct Hash_node{
	void *val;
	struct Hash_node *next;
} Hash_node;



typedef struct HashTable{
	int size;
	int (*hash)(void *);
	Hash_node **entries; //save memory for empty slots by using two pointers
	int (*free_func)(void *); //method to free the values inside nodes
	int (*comp_func)(void *,void *); //method to compare two values
} HashTable;

HashTable *HashTable_init(int size, 
			  int (*hash_func) (void *), int (*free_func) (void *),
			  int (*comp_func) (void *,void *));

int Hash_foreach(HashTable *table, int (*func) (void*,int));
int Hash_install(HashTable *table, void *value);


/*
 * returns a pointer to the value that is the same 
 * as the parameter according to the comp function
 */
void *Hash_get(HashTable *table, void *value);

/*
* removes the first (!) occurence of the value to be found and returns it
* table: 
* value: the value to be found
* returns pointer to the value if it was found
* returns Null if value was not found or error occurs
*/
void *Hash_remove_get(HashTable *table,void *value);

/*
 * removes the value from the table and frees it 
 * returns 1 if the value was found
 */
int Hash_remove(HashTable *table,void *value);

int Hash_comp_int(void *a, void *b);

/*
* completely frees the HashTable
*
*/
void HashTable_free(HashTable  *table);

int Hash_free_int(void *entry);

#define HashTable_int_init() HashTable_init(HASH_FUNC_EXAMPLE_SIZE , &example_hash, &Hash_free_int, &Hash_comp_int)

#ifdef __cplusplus 
}
#endif
#endif
