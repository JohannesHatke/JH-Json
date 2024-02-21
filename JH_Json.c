#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "ArrayList/ArrayList.h"
#include "HashTable/HashTable.h"
#include "UTF8/utf8.h"
#include "JH_Json.h"

#define MAX_SAFE_STR 100
#define MAXLINE 1000

// error handling

void die(){
	//do some cleanup
	printf("depreceated die() method\n");
	exit(1);
}


/* Hash Table */




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

void json_val_free(json_val *p);

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




#define HashTable_Json_init() HashTable_init(JSON_HASH_SIZE, &json_hash_func,&free_json_obj, &json_obj_comp)

int print_json_obj(void *a){
	json_obj *ja = a;
	printf("%s : ?\n",ja->key);
	return 0;
}




/* string buffer and utility input functions*/


#define SBUF_INCREASE 100
typedef struct sbuf {
	char *str;
	int allocated;
}
sbuf;
void sbuf_free(sbuf *sb){
	free(sb->str);
	free(sb);
}

sbuf *sbuf_init(){
	sbuf *output = (sbuf*) malloc(sizeof(sbuf));
	output->str = (char*) malloc(SBUF_INCREASE);
	*(output->str) = '\0';
	output->allocated = SBUF_INCREASE;
	return output;
}
void sbuf_append(sbuf *buf, char *p){
	int newsize = strlen(p) + strlen(buf->str)+1;
	char *tmp;
	if (newsize > buf->allocated){
		buf->allocated = ((newsize / SBUF_INCREASE) + 1 )*SBUF_INCREASE;
		tmp = malloc(buf->allocated);
		strncpy(tmp,buf->str,buf->allocated);
		free(buf->str);
		buf->str = tmp;
	}
	strcat(buf->str,p);
}

#define line_is_empty(s) (strncmp(s,"\n",MAXLINE) == 0)

/*
* skip whitespace
*/
void skipwhitespace( char **p){
	while( isspace(**p))
		(*p)++;
}


/* JSON Part */


int err_val = JSON_NUM;


/* Json_val methods for cleanup */


void AL_free_wrapper(void *p, int pos){
	json_val_free((json_val*) p);
}

void json_val_free(json_val *p){
	switch( p->type){
		case JSON_LIST:
			AL_foreach((ArrayList*) p->data, &AL_free_wrapper);
			AL_free((ArrayList*) p->data);
		break;
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
		case JSON_OBJ:
			HashTable_free((HashTable*) p->data);
		break;
		default:
			fprintf(stderr,"trying to free an unsupported json value of type %d\n",p->type);
			die();
		break;
	}
	free(p);
}

/* Json Printing */


void fprint_json(FILE *output, json_val* jv);
void print_json_val(FILE *output, json_val *jv);

int Nesting = 0; /* TODO: remove global variables*/
FILE *Out;
#define TABSIZE 8

void print_json_str(FILE *output, char *s){
	int c; fprintf(output,"\"");
	while (( c = *s++) != '\0' ){
		switch(c){
			case '"':
				fprintf(output,"\\\"");
			break;
			case '\\':
				fprintf(output,"\\\\");
			break;
			case '/':
				fprintf(output,"\\/");
			break;
			case '\b':
				fprintf(output,"\\b");
			break;
			case '\f':
				fprintf(output,"\\f");
			break;
			case '\n':
				fprintf(output,"\\n");
			break;
			case '\r':
				fprintf(output,"\\r");
			break;
			case '\t':
				fprintf(output,"\\t");
			break;
			default:
				fprintf(output,"%c",c);
			break;
		}
	}
	fprintf(output,"\"");
}

void print_nesting(FILE *output, int n){
	while(n--)
		fprintf(output,"\t");
}

void print_AL_entry(void *p, int pos){
	json_val *jv = (json_val*) p;
	fprintf(Out,"%s\n",( pos != 0) ? "," : "");
	print_nesting(Out, Nesting);
	print_json_val(Out,jv);
}

int print_json_hash_entry(void *v,int pos){
	json_obj *a = (json_obj*) v;

	json_val *jv = a->val;
	fprintf(Out,"%s\n",( pos != 0) ? "," : "");

	print_nesting(Out, Nesting);
	fprintf(Out,"\"%s\":",a->key);
	print_json_val(Out,jv);
	return 0;
}

void print_json_val(FILE *output, json_val *jv){
	switch(jv->type){
		case JSON_STR:
			print_json_str(output,(char*) jv->data);
		break;
		case JSON_NUM:
			fprintf(output,"%f",*((float*) jv->data));
		break;
		case JSON_LITERAL:
			fprintf(output,"%s",(*((int*) jv->data) == -1 ) ? "null" :  ((*((int*) jv->data) ) ? "true" : "false") );
		break;
		case JSON_LIST:
			fprintf(output,"[");
			Nesting++;

			ArrayList *ls = jv->data;
			if ( AL_foreach(ls,&print_AL_entry) == 0){
				Nesting--;
				fprintf(output,"]");
				break;
			}

			Nesting--;
			fprintf(output,"\n");
			print_nesting(Out, Nesting);
			fprintf(output,"]");
		break;
		case JSON_OBJ:
			fprintf(output,"{");
			Nesting++;

			Hash_foreach((HashTable*) jv->data, &print_json_hash_entry);

			Nesting--;
			fprintf(output,"\n");
			print_nesting(Out, Nesting);
			fprintf(output,"}");
		break;
		default:
			fprintf(output,"unknown\n");
		break;
	}
}


void fprint_json(FILE *output, json_val *jv){
	Out = output;
	Nesting = 0;
	print_json_val(output, jv);
	fprintf(output,"\n");
}



/* parsing methods */
json_val *parse_val(char **after);
json_val *parse_object(char **after);
json_val *parse_literal(char **after);



char *read_str(char **p);
json_val *parse_object(char **after){
	err_val = JSON_OBJ;
	json_val *output, *val= NULL;
	char *key = NULL;
	int iteration = 0;
	HashTable *object_table = HashTable_Json_init();

	if(**after == '}'){
		output = malloc(sizeof(json_val));
		output->type = JSON_OBJ;
		output->data = object_table;
		(*after)++;
		return output;
	}



	while(**after != '\0' && **after != '}'){

		skipwhitespace(after);
		key = NULL;
		if(**after == '"'){
			(*after)++;
			key = read_str(after);
		}

		if( key == NULL){
			HashTable_free(object_table);
			return NULL;
		}

		skipwhitespace(after);
		if (**after != ':'){
			free(key);
			HashTable_free(object_table);
			return NULL;
		}
		(*after)++;
		skipwhitespace(after);
		val = NULL;
		val = parse_val(after);
		if (val == NULL){
			free(key);
			HashTable_free(object_table);
			return NULL;
		}

		//add to table
		json_obj *new = malloc(sizeof(json_obj));
		new->key = key;
		new->val = val;
		Hash_install(object_table,new);

		skipwhitespace(after);
		if (**after == ',')
			(*after)++;
		else if (**after == '}'){
			(*after)++;
			break;
		} else {
			HashTable_free(object_table);
			return NULL;
		}
		

	}

	output = (json_val*) malloc(sizeof(json_val));
	output->data = (void*) object_table;
	output->type = JSON_OBJ;
	return output;

}




json_val *parse_literal(char **s){
	int *data = malloc(sizeof(int));
	json_val *output;
	switch(**s){
		case 't':
			if (strncmp(*s,"true",4) == 0){
				*data = JSON_TRUE;
				*s = *s +4;
			}
		break;
		case 'f':
			if (strncmp(*s,"false",5) == 0){
				*data = JSON_FALSE;
				*s = *s +5;
			}
		break;
		case 'n':
			if (strncmp(*s,"null",4) == 0){
				*data = JSON_NULL;
				*s = *s +4;
			}
		break;
		default:
			free(data);
			err_val = JSON_LITERAL;
			return NULL;
		break;
	}

	skipwhitespace(s);
	output = malloc(sizeof(json_val));
	output->data = data;
	output->type = JSON_LITERAL;
	return output;
}

json_val *parse_list(char **after){
	#define skipline(c) if (**after == c && *((*after)+1) != '\0') (*after)++
	json_val *output = NULL;
	json_val *tmp = NULL;
	int failed = 0;
	ArrayList *al = AL_init(10);
	int i = 0;
	skipwhitespace(after);
	while ( **after != ']'){
		if (**after != ',' && i != 0){
			failed = 1;
		}
		skipwhitespace(after);
		skipline(',');
		skipwhitespace(after);

		if (**after != ']') //without we try to parse ] as val
			tmp = parse_val(after);
		else {
			failed =1;
			break;
		}

		if (tmp == NULL){
			failed = 1; //encountered , at end of list
			break;
		}
		AL_append(al,tmp);
		skipwhitespace(after);
		i++;
	}

	if (failed){
		AL_foreach(al, &AL_free_wrapper);
		AL_free(al);
		err_val = JSON_LIST;
		return NULL;
	}
	(*after)++; // skipping ]
	output = malloc(sizeof(json_val));
	output->data = (void*) al;
	output->type = JSON_LIST;
	return output;
}

json_val *parse_num(char **after){
	// not using sscanf because we have to move the pointer anyway
	if ( **after == '0' &&  isdigit(*(*(after)+1))){
		err_val = JSON_NUM;
		return NULL;
	}

	float val = 0;
	int sign = 1;
	float frac = 0.1;
	int exp = 0;
	int expsign = 1;
	float full_exponent = 1;
	if (**after == '-'){
		after++;
		sign = -1;
	}
	while (isdigit((**after))){
		val += val * 10 + **after - '0';
		(*after)++;
	}
	if ( **after == '.'){
		(*after)++;
		while (isdigit((**after))){
			val += (**after - '0') * frac;
			frac *= 0.1;
			(*after)++;
		}
	}
	if ( **after == 'e' || **after == 'E'){
		(*after)++;
		
		if (**after == '-' || **after == '+'){
			expsign = (**after == '.') ? -1 : 1;
			after++;
		}
		while (isdigit((**after))){
			exp += exp *10 + **after - '0';
			(*after)++;
		}
	} else {
		exp = 0;
	}

	while (exp--)
		full_exponent *= 10;
	
	if ( expsign == -1)
		full_exponent = 1 / full_exponent;
	
	json_val *output = malloc(sizeof(json_val));
	output->data = (void*) malloc(sizeof(float));
	output->type = JSON_NUM;
	*((float*) output->data) = sign*(val * full_exponent);

	if(**after == '\n')
		(*after)++;
	return output;
}

int four_digit_hex_to_int(char *p){
	int i,charval,val=0,c;
	for (i =0 ; i<4 && *p != '\0'; i++) {
		c = *p++;
		if (isdigit(c)){
			charval = c - '0';
		} else if (isalpha(c)){
			c = (isupper(c)) ? c + 32: c;
			
			charval = c - 'a' + 10;
		} else {
			return -1;
		}
		val = val *16 + charval;
	}
	if ( i != 4) {
		return -1;
	}
	return val;
	
}

/*
* starts inside string already
*/
char *read_str(char **p){
	int unic,fail,c,escaped = 0;
	fail = 0;
	sbuf *s = sbuf_init();
	char *str,storage[2];
	char *tmp;
	storage[1] = '\0';
	while( (c = (**p) ) != '\0' && !(c == '"' && !escaped)){
		(*p)++;
		if(!escaped &&  c == '\\'){
			escaped = 1;
			continue;
		}
		// TODO fix escaped characters not being recognised
		if (escaped) {
			escaped = 0;
			switch(c){
				case '"':
					sbuf_append(s, "\"");
				break;
				case '\\':
					sbuf_append(s, "\\");
				break;
				case '/':
					sbuf_append(s, "/");
				break;
				case 'b':
				       sbuf_append(s, "\b");
				break;
				case 'f':
				       sbuf_append(s, "\f");
				break;
				case 'n':
				       sbuf_append(s, "\n");
				break;
				case 'r':
				       sbuf_append(s, "\r");
				break;
				case 't':
					sbuf_append(s, "\t");
				break;
				case 'u':
					if ( (unic = four_digit_hex_to_int(*p)) < 0){
						fail = 1;
						break;
					}
					*p = *p + 4;
					tmp = utf8_encode(unic);
					sbuf_append(s,tmp);
					free(tmp);
				break;
				default:
					//TODO write error (for now just skipping
				break;
			}
		} else {
			storage[0] = c;
			sbuf_append(s, storage);
		}
	}
	if (c != '"' || fail){
		sbuf_free(s);
		return NULL;
	}
	

	(*p)++;
	str = s->str;
	free(s);
	return str;
}

json_val *parse_str(char **p){
	char *str = read_str(p);
	json_val *output;
	output = malloc(sizeof(json_val));
	err_val = output->type = JSON_STR;
	output->data = str;
	return output;
}

json_val *parse_val(char **after){
	skipwhitespace(after);
	json_val *output = NULL;
	switch(**after){
		case '[':
			(*after)++;
			output = parse_list(after);
		break;
		case '{':
			(*after)++;
			output = parse_object(after);
		break;
		case '"':
			(*after)++;
			output = parse_str(after);
		break;
	}

	if (!output && strchr("-0123456789",**after) != NULL)
		return parse_num(after);


	if (!output && ((output = parse_literal(after)) == NULL)){
		;
	}

	skipwhitespace(after);
	return output;
}


void json_err(char *begin, char *end){
	const char *err_codes[] = {
	"NUM_ERR",
	"CHAR_ERR",
	"STR_ERR",
	"OBJ_ERR",
	"LITERAL_ERR",
	"LIST_ERR",
	"VAL_ERR"
	};
	char *linebegin;
	char *curr = linebegin = begin;
	int len,offset,linenum = 1;
	offset= 0;
	while ( curr != end){
		if(*curr == '\n'){
			linebegin = curr+1;
			offset = -1;
			linenum++;
		}
		curr++;
		offset++;
	}

	curr = linebegin;
	len = 2;
	while ( *curr != '\0' && *curr != '\n'){
		curr++;
		len++;
	}
	const char *gap = "          ";
	char err_msg[100+len+offset*3];

	fprintf(stderr,"%d\n",err_val-JSON_NUM);
	sprintf(err_msg,"%s occured on line %d:\n",err_codes[err_val- JSON_NUM],linenum);
	strncat(err_msg,linebegin,len-2);
	strcat(err_msg,"\n");
	for (int i = 0; i< offset / 10; i++) {
		strcat(err_msg,gap);
	}
	for (int i = 0; i< offset % 10; i++) {
		strcat(err_msg," ");
	}
	strcat(err_msg,"^\n");

	fprintf(stderr,"%s",err_msg);
}

json_val *parse_json_from_str(char *s){
	char **str = malloc(sizeof(char*));
	*str = s;
	char *before = *str;
	json_val *output;
	if ((output = parse_val(str)) == NULL){
		printf("\n\n\t%.5s\n\n",*str);
		json_err(before,*str);
	}
	free(str);
	return output;
}


json_val *parse_json_file(char *filename){
	json_val *output;
	char *buf;  

	FILE *input = fopen(filename,"r");
	if (input == NULL){
		fprintf(stderr,"cant find file %s\n",filename);
		free(buf);
		return NULL;
	}

	/* SEEK_END is not guaranteed */

	#ifndef SEEK_END 
	buf  = (char*) malloc(sizeof(char) * MAXLINE);
	sbuf *sb = sbuf_init();

	//TODO: improve to allow infinite line length using sbuf
	while ( fgets(buf, MAXLINE, input) != NULL){
		if (line_is_empty(buf))
			continue;
		sbuf_append(sb, buf);
		
	}
	fprintf(stderr,"file input read:\n%s",sb->str);

	fprintf(stderr,"begin parsing\n");
	output = parse_json_from_str(sb->str);
	sbuf_free(sb);
	free(buf);

	#else 

	fseek(input, 0, SEEK_END);
	int length = ftell(input) +1;
	fseek(input, 0, SEEK_SET);
	if (length < 0){
		fprintf(stderr,"Error getting file length of %s\n",filename);
		return NULL;
	}
	buf = malloc(length);
	if ( buf == NULL ){
		fprintf(stderr,"Memory Error");
		return NULL;
	}
	if (  fread(buf, 1,length, input) < 0 ){
		fprintf(stderr,"Error reading file");
		free(buf);
		return NULL;
	}
	buf[length-1]='\0';
	output = parse_json_from_str(buf);
	free(buf);
	#endif /* ifndef SEEK_END  */
	fclose(input);
	return output;
}


// json-wrappers




// dictionaries:

/*
* gets the value from the dictionary
* returns NULL if error occurs or the value could not be found 
*/
json_val* json_dict_get(json_val *dict, char *key){
	if (dict->type != JSON_OBJ){
		fprintf(stderr, "json: dict-get: called object is not a dictionary\n");
		return NULL;
	}
	return Hash_get((HashTable*) dict->data, (char *) key);
}

/*
* deletes the value from the dictionary
* returns non-zero if value was found
*/
int json_dict_delete(json_val *dict, char *key){
	if (dict->type != JSON_OBJ){
		fprintf(stderr, "json: dict-delete: called object is not a dictionary\n");
		return 0;
	}
	return Hash_remove((HashTable*) dict->data, (char *) key);
}

/*
* removes the value from the dictionary and returns it
* returns NULL if not found
*/
json_val *json_dict_pop(json_val *dict, char *key){
	if (dict->type != JSON_OBJ){
		fprintf(stderr, "json: dict-pop: called object is not a dictionary\n");
		return 0;
	}
	return Hash_remove_get((HashTable*) dict->data, (char *) key);
}

/*
* sets the value
* returns:
*	0 if an error occurs
*	1 if there was no  previous value with the key
*	2 if there was a previous value with the key
*/
int json_dict_set(json_val *dict, char *key, json_val *val){
	if (dict->type != JSON_OBJ){
		fprintf(stderr, "json: dict-set: called object is not a dictionary\n");
		return 0;
	}
	if (val == NULL || key == NULL ){
		fprintf(stderr, "json: dict-set: parameters are NULL. Use JSON_NULL \n");
		return 0;
	}

	int found = Hash_remove((HashTable*) dict->data, (char *) key);
	Hash_install((HashTable*) dict->data, (char *) key);
	return found + 1;
}

// List

int json_list_set(json_val *arr, int index, json_val *val){
	if (arr->type != JSON_LIST){
		fprintf(stderr, "json: list-set: called on json object, that is not a list\n");
		return 0;
	}
	if (val == NULL || index < 0 ){
		fprintf(stderr, "json: list-set: wrong index or value\n");
		return 0;
	}
	AL_set( (ArrayList*) arr, index, val);
	return 1;
}
json_val *json_list_get(json_val *arr, int index){
	if (arr->type != JSON_LIST){
		fprintf(stderr, "json: list-get: called on json object, that is not a list\n");
		return NULL;
	}
	if (index < 0 ){
		fprintf(stderr, "json: list-get: wrong index\n");
		return NULL;
	}
	return AL_get( (ArrayList*) arr, index);
}

// TODO: restructure ArrayList so values can be deleted without shifting everything

