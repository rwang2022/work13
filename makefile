all: work13.o
	gcc -o program work13.o
work13.o: work13.c
	gcc -c work13.c
run: 
	./program
clean: 
	rm *.o
	rm program