main:main.c utf8
	rm -r build
	mkdir build
	mv ./utf8.o ./build
	gcc -pedantic -Wall -c main.c -lm
	mv ./main.o ./build
	gcc ./build/utf8.o ./build/main.o -o main -lm

utf8:utf8.o
	gcc -pedantic -Wall -c -o utf8 utf8.c -lm
