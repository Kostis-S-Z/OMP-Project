all:
    generate
    compile
    ./project

clean:
    rm *.o

generate:
    ./generator input

compile:
    gcc -Wall main.c -o project
