all:
	make generate
	make examine
clean:
	rm -f *.out
	rm -f *.o
generate:
	gcc -Wall generator.c -o NumGenerator.out
	./NumGenerator.out datafile.out 1500

examine:
	gcc -Wall main.c -o project.out
	./project.out -1 -1 datafile.out -1 -1




