./build/main:build ./build/main.o ./build/utf8.o
	cc -pedantic -g -Wall -o ./build/main ./build/main.o
./build: 
	mkdir build
./build/main.o: ./build main.c
	cc -pedantic -g -Wall -c -o ./build/main.o main.c
./build/utf8.o: ./build ./UTF8/utf8.c
	# no warnings because overflow is intended
	cc  -g -c -o ./build/utf8.o ./UTF8/utf8.c
clean: 
	rm -r ./build
