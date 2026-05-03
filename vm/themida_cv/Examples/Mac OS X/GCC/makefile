CC=gcc

all: test_x86 test_x64

test_x86: main.c
	$(CC) -o test_x86 main.c -arch i386 

test_x64: main.c
	$(CC) -o test_x64 main.c -arch x86_64 

clean:
	rm *.o test_x64 test_x86


