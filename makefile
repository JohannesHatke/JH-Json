CFLAGS= -Wall -Wextra -pedantic -O0 -g

lib: build ./build/JH_Json.o ./build/utf8.o ./build/ArrayList.o ./build/HashTable.o 
	mkdir lib
	cp ./build/JH_Json.o ./lib/
	cp ./JH_Json.h ./lib/

tests: build ./build/JH_Json.o ./build/utf8.o ./build/ArrayList.o ./build/HashTable.o tests.c
	$(CC) $(CFLAGS)  ./build/utf8.o ./build/ArrayList.o ./build/HashTable.o  ./build/JH_Json.o tests.c -o ./build/tests -lm

./build: 
	mkdir build
./build/JH_Json.o: ./build JH_Json.c
	$(CC) $(CFLAGS) -c -o ./build/JH_Json.o JH_Json.c
./build/utf8.o: ./build ./UTF8/utf8.c
	# no warnings because overflow is intended
	$(CC) -g -c -o ./build/utf8.o ./UTF8/utf8.c
./build/ArrayList.o: ./ArrayList/ArrayList.c
	$(CC) $(CFLAGS) -c ./ArrayList/ArrayList.c -o ./build/ArrayList.o 
./build/HashTable.o: ./HashTable/HashTable.c
	$(CC) $(CFLAGS) -c ./HashTable/HashTable.c -o ./build/HashTable.o 

clean: 
	rm -r ./build
	rm -r ./lib
