#include <stdio.h>
#include "../JH_Json.h"

// general test setup:
int main(int argc, char **argv){
	// freopen("/dev/null", "w", stderr);

	if( argc < 2){
		printf("no args\n"); 
		return 1;
	}
	
	json_val *test = json_read_file(argv[1]);
	if (test == NULL){
		printf("failure\n"); 
		return 2;
	}
	//fprintf(stderr,"----\nparsed: \n");
	//fprintf_json(stderr,test);
	printf("success\n"); 
	json_val_free(test);

	return 0;
}
