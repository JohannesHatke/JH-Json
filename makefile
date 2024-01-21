CFLAGS= -Wall -Wextra -pedantic -O0 -g

./build/main:build ./build/main.o ./build/utf8.o ./build/ArrayList.o ./build/HashTable.o
	$(CC) $(CFLAGS)  ./build/utf8.o ./build/ArrayList.o ./build/HashTable.o  ./build/main.o -o ./build/main -lm
./build: 
	mkdir build
./build/main.o: ./build main.c
	$(CC) $(CFLAGS) -c -o ./build/main.o main.c
./build/utf8.o: ./build ./UTF8/utf8.c
	# no warnings because overflow is intended
	$(CC) -g -c -o ./build/utf8.o ./UTF8/utf8.c
./build/ArrayList.o: ./ArrayList/ArrayList.c
	$(CC) $(CFLAGS) -c ./ArrayList/ArrayList.c -o ./build/ArrayList.o 
./build/HashTable.o: ./HashTable/HashTable.c
	$(CC) $(CFLAGS) -c ./HashTable/HashTable.c -o ./build/HashTable.o 

clean: 
	rm -r ./build
