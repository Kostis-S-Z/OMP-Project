all:
	make generate
	make examine
clean:
	rm -f *.out
	rm -f *.o
generate:
	gcc -Wall generator.c -o NumGenerator.out
	./NumGenerator.out datafile.out 15000000

examine:
	mpicc -fopenmp -Wall parallelOMP.c -o OMPproject.out
	mpirun ./OMPproject.out -1 -1 datafile.out -1 -1

linear: 
	gcc -Wall linear.c -o linearProject.out
	./linearProject.out -1 -1 datafile.out -1 -1




