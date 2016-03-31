#!/bin/bash
set autoscale
set term png
#set term latex
set output "times.png"
#set output "times.tex"

unset key

set yrange [0:11]
set xrange [-0.1:5.5]

set boxwidth 0.1
set style fill solid

set title "Running times for 15 million collisions"
set ylabel "Seconds"
set xtics ("Linear" 0.5, "Using OMP" 2, "Using MPI" 3.5, "Using OMP and MPI" 5)
set ytics 0,1,11

plot 'lineardata.dat' with boxes lt rgb "red", 'ompdata.dat' with boxes lt rgb "orange", 'mpidata.dat' with boxes lt rgb "green", 'paralleldata.dat' with boxes lt rgb "blue"
