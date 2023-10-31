CFLAGS= -Wall -Wextra -pedantic -std=c99 -g
main: ArrayList.o main.o
	$(CC) $(CFLAGS) main.o ArrayList.o -o main 
ArrayList.o: ArrayList.c
	$(CC) $(CFLAGS) -c ArrayList.c -o ArrayList.o -Wall 
main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.o 
clean:
	rm ArrayList.o
	rm main.o
	rm main
