major2: major2.o
	gcc major2.o -o major2
major1.o: major2.c major2.h
	gcc -c- Wall major1.c - lm
clean:
	rm -f *.o major2
