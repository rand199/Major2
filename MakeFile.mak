
all: Major2

Major2: 
gcc major2.c -o main.o

Interactive Mode:
./main.o
Batch Mode:
./main.o batchfile.txt
clean:  
rm main.o