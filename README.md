# Small Json Parser
This is a small json parser that tries to be compliant with [ECMA-404](https://ecma-international.org/publications-and-standards/standards/ecma-404/). I wrote this mainly for fun to be able to use json in other C Projects of mine. It has basic features for parsing a json file to use the data. Theres also functionality to create new json files and update existing ones.

## important limitations
The maximum length of a string in a parsed json file is 100 000 bits in this implementation. I opted against using a variable length string here to increase performance on large files. This upper limit is set with the MAX_SAFE_STR definition at compile time.

## Data structures
Every piece of data in the json file is represented as a value in form of the _json_val_ struct. These structs have a type member and a data member. The data member is a void pointer that can be cast depending on the type:

| Type   | Data    |
|--------------- | --------------- |
| JSON_NUM      | Float   |
| JSON_STR	    | char*   |
| JSON_LITERAL	| integer constant that can be used as a bool  |
| JSON_OBJ	    | A HashList object (see below)   |
| JSON_LIST	    | An ArrayList object (see below)   |

### Json Literals
The possible value of a json literal are JSON_NULL, JSON_FALSE and JSON_TRUE. When using these as a boolean value it is important to know that JSON_NULL will evaluate as true, while JSON_FALSE and JSON_True will behave as expected.

### json objects
There are a couple of methods to access the members of the hashtable: 
```C
json_val* json_dict_get(json_val *dict, char *key);
int json_dict_delete(json_val *dict, char *key);
json_val *json_dict_pop(json_val *dict, char *key);
int json_dict_set(json_val *dict, char *key, json_val *val);
```

### Json Lists
```C
int json_list_set(json_val *arr, int index, json_val *val);
json_val *json_list_get(json_val *arr, int index);
```

### general methods
```C
- json_val *json_val_create(int type, void *value);
- void json_val_free(json_val *p);
- void json_fprintf(FILE *output, json_val* jv);
- void json_printf(k);
- json_val *json_read_file(char *filename);
- json_val *json_read_str(char *str);
```

(Almost all these functions and their return values are explained in the header file.)

## Testing
There are a number of tests included which you can check out by running `make tests && ./tests/run.py`. This tries to use valgrind to check for memory leaks. If you want to disable checking for leaks add the option `--no-memcheck`. 

## Installing
Because this is kind of work in progress I have not added a way to add this to the os automatically. Instead I would recommend just linking it yourself after compiling it by running `m̀ake lib`. The object and header file will be in the created lib directory.
