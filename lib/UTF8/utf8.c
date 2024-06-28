
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

int utf8_strlen(char *p) {
	int i = 0;
	int j = 0;
	while(*p != '\0'){
		j = 1;
		if (( (unsigned) *p ) > 0x0080)
			while (*(p+j) != '\0' && ((unsigned) *(p+j)) >= 0x0080 && j < 5){
				j++;
			}

		p = (p+j);
		i++;
	}
		
	return i;

}


//position indexed 0: starting with least significant bit
void setbit(char *a,int pos,int val){
	unsigned char tmp = 0;
	tmp = pow(2,pos);
	//printf("tmp = %d\n",tmp);
	if (val == 0){
		*a = *a & ~tmp;
		return;
	}
	*a = *a | tmp;

}

static int checkbit(char a,int pos){
	a = a >> pos;
	a = a & 1;
	return a;
}

static void examinebits(char a){
	for (int i =7 ;  i > -1; i--)
		printf("%d",checkbit(a, i));
	printf("\n");
}

//returns malloced string
char *utf8_encode(int val){
	char *output = NULL;
	if( val < 0 || val > 0x10FFF)
		return NULL;

	if(val < 0x0080){
		output = malloc(2 * sizeof(char));
		output[0] = val;
		output[1] = '\0';
	}
	else if(val < 0x0800){
		output = malloc(3 * sizeof(char));
		output[0] = 0xC0;
		output[1] = 0x80;
		output[2] = '\0';

		output[1] = output[1] | (0x3F & val);
		val = val >> 6;
		output[0] = output[0] | (0x1F & val);
	}
	else if(val < 0x10000){
		output = malloc(4 * sizeof(char));
		output[0] = 0xE0;
		output[1] = output[2] =  0x80;
		output[3] = '\0';

		for(int i = 2; i >0; i--){
			output[i] = output[i] | (0x3F & val);
			val = val >> 6;
		}
		output[0] = output[0] | (0x0F & val);
	}
	else if(val < 0x10FFF){
		output = malloc(5 * sizeof(char));
		output[0] = 0xF0;
		output[1] = output[2] = output[3] = 0x80;
		output[4] = '\0';

		for(int i = 3; i >0; i--){
			output[i] = output[i] | (0x3F & val);
			val = val >> 6;
		}
		output[0] = output[0] | (0x07 & val);
	}
	return output;
		
}
long utf8_decode(char *p){
	int i,len;
	char c = *p; //copying first byte so we dont need to worry about string literals
	unsigned long int val;
	if (checkbit(c,7) == 0)
		return *p;
	if (checkbit(c,6) == 0) //middle of utf8 char
		return 0;

	//examinebits(c);
	for (i = 7; i > 3 && checkbit(c,i) == 1; i--)
		setbit(&c,i,0);

	if (i == 3 && checkbit(c,3) != 0)
		return 0; //invalid
	len = 7-i;

	val = c;
	//examinebits(val);
	while(len-- > 1 && *++p != '\0'){
		//printf("len: %d\n",len);
		//printf(" Byte : %x\n",c);
		if ( checkbit(*p,7) != 1 || checkbit(*p,6) != 0)
			return 0;
		val *= 64;
		//printf("val = %ld\n",val);
		//examinebits(*p);
		//examinebits(*p & ~0xC0);
		val += (char) (*p & ~0xC0); //strips first two bits
	}
	//printf("output %ld",val);
	return val;
}

