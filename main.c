#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "ArrayList/ArrayList.h"
#include "UTF8/utf8.h"

#define MAX_SAFE_STR 100
#define MAXLINE 1000

// error handling

void die(){
	//do some cleanup
	printf("depreceated die() method\n");
	exit(1);
}

/* Data Structures: */

/* Hash Table */

/*
 * using dbj2 but reducing value significantly, only fast for few values
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


int hash_foreach( hash_obj **table,  void (*fp) (hash_obj*, int) ){
	int i,count;
	count = i = 0;
	hash_obj *curr,*old;
	for(i = 0; i< HASHSIZE; i++  ){
		curr = table[i];
		while(curr != NULL){
			old = curr;
			curr = curr->next;
			(*fp)(old,count);
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

enum lit_token {
	JSON_NULL = -1,
	JSON_FALSE,
	JSON_TRUE
};



typedef struct json_val{
	void *data;
	int type;
}json_val;




#define KEY 10;
#define VAL 11;

enum val_type {
	JSON_NUM = 2000,
	JSON_CHAR,
	JSON_STR,
	JSON_OBJ,
	JSON_LITERAL,
	JSON_LIST,
	JSON_VAL //just for error checking

};

int err_val = JSON_NUM;


/* Json_val methods for cleanup */

void json_val_free(json_val *p);
void hash_free_wrapper(hash_obj *obj, int pos){
	json_val_free((json_val *)obj->val);
}

void free_hash_contents(hash_obj **table){
	hash_foreach(table, &hash_free_wrapper);
}

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
			//hash_foreach((hash_obj**) p->data, &hash_free_wrapper);
			free_hash_table( (hash_obj**) p->data);
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
	int c;
	fprintf(output,"\"");
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
void print_hash_entry(hash_obj *p, int pos){
	json_val *jv = (json_val*) p->val;
	fprintf(Out,"%s\n",( pos != 0) ? "," : "");
	print_nesting(Out, Nesting);
	fprintf(Out,"%s\n",p->name);
	print_json_val(Out,jv);
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

			hash_obj **table = (hash_obj **) jv->data;
			if ( hash_foreach(table,&print_hash_entry) == 0){
				Nesting--;
				fprintf(output,"}");
				break;
			}

			Nesting--;
			fprintf(output,"\n");
			print_nesting(Out, Nesting);
			fprintf(output,"}");
		break;
		//TODO json_obj
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
	hash_obj **table = init_hash_table();



	while(**after != '\0' && **after != '}'){

		skipwhitespace(after);
		key = NULL;
		if(**after == '"'){
			(*after)++;
			key = read_str(after);
		}

		if( key == NULL){
			free_hash_table(table);
			return NULL;
		}

		skipwhitespace(after);
		if (**after != ':'){
			free(key);
			free_hash_table(table);
			return NULL;
		}
		(*after)++;
		skipwhitespace(after);
		val = NULL;
		val = parse_val(after);
		if (val == NULL){
			free(key);
			free_hash_table(table);
			return NULL;
		}

		//add to hashlist
		install(table, key ,(void*) val);

		skipwhitespace(after);
		if (**after == ',')
			(*after)++;
		else if (**after == '}')
			break;
		else {
			free(val);
			free(key);
			free_hash_table(table);
			return NULL;
		}
		

	}
	output = (json_val*) malloc(sizeof(json_val));
	output->data = (void*) table;
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
		json_err(before,*str);
	}
	printf("3%s\n",before);
	printf("3%s\n",*str);
	free(str);
	return output;
}


json_val *parse_json_file(char *filename){
	json_val *output;
	char *line = (char*) malloc(sizeof(char) * MAXLINE);

	FILE *input = fopen(filename,"r");
	if (input == NULL){
		fprintf(stderr,"cant find file %s\n",filename);
		free(line);
		return NULL;
	}

	sbuf *sb = sbuf_init();

	//TODO: improve to allow infinite line length using sbuf
	while ( fgets(line, MAXLINE, input) != NULL){
		if (line_is_empty(line))
			continue;
		sbuf_append(sb, line);
		
	}
	fprintf(stderr,"file input read:\n%s",sb->str);

	fprintf(stderr,"begin parsing\n");
	output = parse_json_from_str(sb->str);
	sbuf_free(sb);
	free(line);
	fclose(input);
	return output;
}



// general test setup:
int main(int argc, char **argv){
	// freopen("/dev/null", "w", stderr);

	if( argc < 2){
		printf("no args\n");
		return 1;
	}
	
	json_val *test = parse_json_file(argv[1]);
	if (test == NULL)
		return 2;
	fprintf(stderr,"----\nparsed: \n");
	fprint_json(stderr,test);
	json_val_free(test);

	return 0;
}



	//In seperate branch:
	// - (2) add objects (using general structure of parse_list
	//	- add tests
	// - (3) add README and proper documentation
	// - (4) add wrappers to get and set data
	// - (5) improve error checking (it gets the line right but not the char usually in messages)
	
	


