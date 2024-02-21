#include <stdio.h>
#include "JH_Json.h"

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
	//fprintf(stderr,"----\nparsed: \n");
	//fprint_json(stderr,test);
	json_val_free(test);

	return 0;
}
