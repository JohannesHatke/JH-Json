#include <string.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#define MAXSTRING 1000



int hexdigit_to_int(char c){
	if ( c >= '0' && c <= '9')
		return c - '0';
	if ( c >= 'a' && c <= 'f')
		return 10 + c - 'a';
	if ( c >= 'A' && c <= 'F')
		return 10 + c - 'A';
	return -1;
}




int unicode_to_String(char *u, char *c1, char *c2){
	// using UTF 8, so we need at most 6 chars
	int n[2];
	char *p,*p_start;
	if (*u != 'u'){
		printf("u is not first char");
		return 1;
	}

	p_start = u;
	p_start++;
	for(p = p_start; *p != '\0' &&  p-p_start < 2; p++ ){
		n[p-p_start] = hexdigit_to_int(*p);
		printf("\t%c:\t %d\t\n",*p,hexdigit_to_int(*p));
	}
	*c1 = n[0] * 16 + n[1];
	printf("c1 = %d * 16 + %d = %d\n",n[0],n[1],*c1);

	p_start = p;
	for(; *p != '\0' &&  p-p_start < 2; p++ ){
		n[p-p_start] = hexdigit_to_int(*p);
		printf("%c:\t %d\n",*p,hexdigit_to_int(*p));

	}
	*c2 = n[0] * 16 + n[1];
	printf("c2 = %d * 16 + %d = %d\n",n[0],n[1],*c2);
	printf("output: %c%c\n",*c1,*c2);
	return 0;
}

/*
 * reads in string WITHOUT replacing escape-sequences
 * @param char* output 
*/
int *read_in_String(char *st, char *output, int maxlen){
	

	return 0;
}

char *parse_String(char *st){
	static char *escapes = "\"\\/bfnrtu";
	char c,*u1,*u2,*p,*out_p;
	u1 = (char*) malloc(1);
	u2 = (char*) malloc(1);
	char *output = malloc(strnlen(st,MAXSTRING)* sizeof(char));
	out_p = output;

	while ((c=*st++) != '\0' && c != '\"'){ 
		printf("read %c\n",c);
		if (c == '\\'){// c = \\ , *st  ="
			printf("escaping %c\n",*(st));
			int i = 0;
			for (p = escapes; *p != '\0' && *p != *st; p++)
				printf("%d,",i++);
			printf("\n");
			if(*p == '\0'){
				// ERR st is not a defined escape char
				printf("ERR %c is not a defined escape char\n",*st);
				free(u1);
				free(u2);
				free(output);
				return NULL;
			}
			//if(*p != 'u'){
			if(*p != 'u'){
				*out_p = *p;
				printf("%d:\tsetting out_p = %c\n",78,*p);
				st++; // c = \\ out_p = "+1
			}
			else { //*st = u
				/*
				unicode_to_String(st, u1, u2);
				printf("got back %c%c\n",*u1,*u2);
				*out_p++ = *u1;
				*out_p = *u2;
				st = st + 5;
				*/
				unicode_to_String(st, u1, u2);
				if (*u1 != '\0') *out_p++ = *u1;
				*out_p = *u2;
				st = st + 5;

			}
			printf("wrote %c\n",*(out_p));
		}
		else {
			*out_p = c;
			printf("%d:\tsetting out_p = %c\n",85,c);
		}
		out_p++;
	}
	*out_p = '\0';
	printf("%d:\tsetting out_p = 0\n",90);
	free(u1);
	free(u2);
	return output;
}

int main(void){

	printf("\u00BF\n");
	char *locale = setlocale(LC_ALL, "");
	printf("locale set to %s\n",locale);
	char *s1 = malloc(sizeof(char)*MAXSTRING);
	*s1 = ' ';
	strcpy(s1,"test");
	printf("s1:\n");
	printf("\t%s\n\t\t%s\n",s1,parse_String(s1));
	char *s2 = malloc(sizeof(char)*MAXSTRING);
	*s2 = ' ';
	strcpy(s2,"test54");
	printf("s2:\n");
	printf("\t%s\n\t\t%s\n",s2,parse_String(s2));
	char *s3 = malloc(sizeof(char)*MAXSTRING);
	*s3 = ' ';
	strcpy(s3,"test\":");
	printf("s3:\n");
	printf("\t%s\n\t\t%s\n",s3,parse_String(s3));
	char *s4 = malloc(sizeof(char)*MAXSTRING);
	*s4 = ' ';
	strcpy(s4,"test\\\" this should stay");
	printf("s4:\n");
	printf("\t%s\n\t\t%s\n",s4,parse_String(s4));




	char c = -2;
	unsigned char uc = -2;
	printf("%d:\n",((int)((char) 127)));
	printf("%d:\n",((int)((char) -1)));
	printf("\n");
	printf("%d:\n",c);
	printf("%d:\n",(char) uc);

	char c1 = 0xC3;
	char c2 = 0xbf;
	printf("\n\n%c%c:\n",c1,c2);

	
	char *teststr = malloc(10);
	teststr = "u12346";
	unicode_to_String(teststr, &c1, &c2);
	printf("\n\n");
	teststr = "uc2ab";
	unicode_to_String(teststr, &c1, &c2);
	printf("\n\n");

	teststr = "u0021";
	unicode_to_String(teststr, &c1, &c2);
	printf("\n\n");
	printf("\n\n complete tests: ");


	char *s6 = malloc(sizeof(char)*MAXSTRING);
	*s6 = ' ';
	strcpy(s6,"\\\" this should stay \' this to (exclamation mark) \\u0021 \" this should go");
	printf("s6:\n");
	printf("\t%s\n\t\t%s\n",s6,parse_String(s6));
	char *s7 = malloc(sizeof(char)*MAXSTRING);
	*s7 = ' ';
	strcpy(s7,"\\\" this should stay \' this to (exclamation mark) \\u03e7123456789");
	printf("s7:\n");
	printf("\t%s\n\t\t%s\n",s7,parse_String(s7));
	printf("\n\n\u00f7f\n");
	return 0;
}
