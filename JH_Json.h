#ifndef JH_JSON
#define JH_JSON


typedef struct json_val{
	void *data;
	int type;
}json_val;

void json_val_free(json_val *p);


enum {
	JSON_NULL = -1,
	JSON_FALSE,
	JSON_TRUE
};

enum {
	JSON_NUM = 2000,
	JSON_CHAR,
	JSON_STR,
	JSON_OBJ,
	JSON_LITERAL,
	JSON_LIST,
	JSON_VAL //just for error checking

};

/* Json_val methods for cleanup */



void json_fprintf(FILE *output, json_val* jv);

#define json_printf(k) fprintf_json(stdout,k)


json_val *json_read_file(char *filename);
json_val *json_read_str(char *filename);

// json-wrappers

// dictionaries:

/*
* gets the value from the dictionary
* returns NULL if error occurs or the value could not be found 
*/
json_val* json_dict_get(json_val *dict, char *key);


/*
* deletes the value from the dictionary
* returns non-zero if value was found
*/
int json_dict_delete(json_val *dict, char *key);


/*
* removes the value from the dictionary and returns it
* returns NULL if not found
*/
json_val *json_dict_pop(json_val *dict, char *key);


/*
* sets the value
* returns:
*	0 if an error occurs
*	1 if there was no  previous value with the key
*	2 if there was a previous value with the key
*/
int json_dict_set(json_val *dict, char *key, json_val *val);
// List

int json_list_set(json_val *arr, int index, json_val *val);

json_val *json_list_get(json_val *arr, int index);


// TODO: restructure ArrayList so values can be deleted without shifting everything

#endif
