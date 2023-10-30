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


// JSON PART

//input handling

#define line_is_empty(s) (strncmp(s,"\n",MAXLINE) == 0)

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




void remove_whitespace(char *p){
	char *p2 = p;
	int inString = -1;
	int escaped = 0;

	while(*p2 != '\0'){
		while (*p2 != '\0' && 
			inString == -1 && 
			( *p2 == ' ' || *p2 == '\t' || *p2 == '\r' ))
			p2++;

		if (*p2 == '"' && !escaped)
			inString *= -1;

		if (!escaped && *p2 == '\\')
			escaped = 1;
		else{
			escaped = 0;
		}
		*p++ = *p2++;
	}
	*p = '\0';
}

// error handling

void print_error_msg(char *line, int linenum, int pos){
	fprintf(stderr,"Encountered an erro while parsing JSON\n");
	fprintf(stderr,"%5d |%s",linenum,line);
	fprintf(stderr,"%7s","|");
	while(pos-- > 0){
		fprintf(stderr," ");
	}
	fprintf(stderr,"^\n");

}

enum lit_token {
	JSON_NULL = -1,
	JSON_FALSE,
	JSON_TRUE
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
/*
 * only works for Char, num, str, and literals
 */



/*
 *  try parsing. 
 *  On failure it will cleanup all allocated memory and exit program
*/
#define KEY 10;
#define VAL 11;

enum val_type {
	JSON_NUM = 2000,
	JSON_CHAR,
	JSON_STR,
	JSON_OBJ,
	JSON_LITERAL,
	JSON_LIST

};

enum TOKENS {
	KEY_VAL_SEP = 20,
	LIST_OPEN,
	LIST_CLOSE,
	OBJECT_OPEN,
	OBJECT_CLOSE,
	NEWLINE
};

void json_val_free(json_val *p);

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
		default:
			fprintf(stderr,"trying to free an unsupported json value of type %d\n",p->type);
			die();
		break;
	}
	free(p);
}

json_val *parse_val(char **after);
json_val *parse_object(char **after);
json_val *parse_literal(char **after);
json_val *parse_object(char **after){return NULL;}


/*
* skip whitespace
*/
void skipwhitespace( char **p){
	while( isspace(**p))
		(*p)++;
}

/*
* starts immediately after [
*/

json_val *parse_literal(char **s){
	int *data = malloc(sizeof(int));
	printf("parse literal got:%s\n\n",*s);
	json_val *output;
	switch(**s){
		case 't':
			if (strncmp(*s,"true",4) == 0){
				printf("\n\n\t\tbef:%c\n\n",**s);
				*data = JSON_TRUE;
				*s = *s +4;
				printf("\n\n\t\taf:%c\n\n",**s);
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
			return NULL;
		break;
	}
	//TODO: move this into parseList (this behaviour should be checked outside of this function
	if (!isspace(**s) && **s != ',' && **s != ']'){
		free(data);
		return NULL;
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
	while ( **after != ']'){
		skipwhitespace(after);
		if (**after != ',' && i != 0){
			printf("comma failure\n");
			return NULL;
		}
		skipwhitespace(after);
		skipline(',');
		skipwhitespace(after);

		if (**after != ']') //without we try to parse ] as val
			tmp = parse_val(after);
		else 
			break;

		if (tmp == NULL){
			failed = 1;
			break;
		}
		AL_append(al,tmp);
		skipwhitespace(after);
		i++;
	}

	if (failed){
		//TODO free arraylist properly using foreach
		AL_foreach(al, &AL_free_wrapper);
		AL_free(al);
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
	if ( **after == '0' &&  isdigit(*(*(after)+1)))
		return NULL;

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
	printf("value is %f\n",*( (float*)output->data));
	printf("offset string is \n%s\n",*after);

	if(**after == '\n')
		(*after)++;
	return output;
}

/*
* starts inside string already
*/
json_val *parse_str(char **p){
	printf("string enter\n");
	printf("%s",*p);
	int c,escaped = 0;
	sbuf *s = sbuf_init();
	char storage[2];
	json_val *output;
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
					//TODO Unicode
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
	(*p)++;
	output = malloc(sizeof(json_val));
	output->type = JSON_STR;
	output->data = s->str;
	free(s);
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


	//try parsing a literal
	if (!output && ((output = parse_literal(after)) == NULL)){
		printf("parsing value failed %s\n",*after);

	}

	skipwhitespace(after);
	printf("after parsing value is remaining :%s:\n",*after);
	return output;
}


json_val *parse_json_str(char *s){
	char **offset = malloc(sizeof(char*));
	*offset = s;
	json_val *output;
	if ((output = parse_val(offset)) == NULL){
		printf("died at %s\n",*offset);
	}
	free(offset);
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
	while ( fgets(line, MAXLINE, input) != NULL){
		//remove_whitespace(line);
		if (line_is_empty(line))
			continue;
		sbuf_append(sb, line);
		
	}
	printf("%s",sb->str);

	printf("begin parsing\n");
	output = parse_json_str(sb->str);
	sbuf_free(sb);
	free(line);
	fclose(input);
	return output;
}


//output 


void fprint_json(FILE *output, json_val* jv);
void print_json_val(FILE *output, json_val *jv);

int Nesting = 0; /* TODO: remove global variables*/
FILE *Out;
#define TABSIZE 8

void print_nesting(FILE *output, int n){
	while(n--)
		fprintf(output,"\t");
}

void print_entry(void *p, int pos){
	json_val *jv = (json_val*) p;
	fprintf(Out,"%s\n",( pos != 0) ? "," : "");
	print_nesting(Out, Nesting);
	print_json_val(Out,jv);
}

void print_json_val(FILE *output, json_val *jv){
	switch(jv->type){
		case JSON_STR:
			fprintf(output,"%s",((char*) jv->data));
		break;
		case JSON_NUM:
			fprintf(output,"%f",*((float*) jv->data));
		break;
		case JSON_LITERAL:
			fprintf(output,"%s",(*((int*) jv->data) == -1 ) ? "null" :  ((*((int*) jv->data) ) ? "true" : "false") );
			//printf("%d\n",*((int*) jv->data));
		break;
		case JSON_LIST:
			//print_nesting(Out, Nesting);
			fprintf(output,"[");
			Nesting++;

			ArrayList *ls = jv->data;
			if ( AL_foreach(ls,&print_entry) == 0){
				Nesting--;
				fprintf(output,"]");
				break;
			}

			Nesting--;
			fprintf(output,"\n");
			print_nesting(Out, Nesting);
			fprintf(output,"]");
		break;
		default:
			printf("unknown\n");
		break;
	}
}


void fprint_json(FILE *output, json_val *jv){
	Out = output;
	Nesting = 0;
	print_json_val(output, jv);
	fprintf(output,"\n");
}


int main(int argc, char **argv){
	//TODO fix parse_literal to pass test 10

	//TODO do proper error checking
	
	


	//freopen("/dev/null", "w", stderr);
	if( argc < 2){
		printf("no args\n");
		return 1;
	}
	
	json_val *test = parse_json_file(argv[1]);
	printf("%p\n",test);
	ArrayList *testList = test->data;
	//AL_foreach(testList, &print_entry);
	fprint_json(stdout,test);

	//json_val_free(test);

	return 0;
}
