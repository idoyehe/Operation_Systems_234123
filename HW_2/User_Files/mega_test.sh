#!/bin/bash
gcc -o test6 hw2_test6.c
mkdir outputs
for i in `seq 1 1000`;
do
	echo "test number $i"
	./test6 $i > outputs/out_$i.txt
done 
