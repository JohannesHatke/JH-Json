main:main.c str
	gcc -pedantic -g -Wall -o main main.c
str:str.c
	gcc -pedantic -Wall -o str str.c

clean: 
	rm ./main
	make main
