
all: major2

Major2: 
gcc major2.c -o main.o

Interactive Mode:
./major2.o

Batch Mode:
./major2.o batchfile.txt

clean:  
rm major2.o
