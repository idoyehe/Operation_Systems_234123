#!/bin/bash
clear
chmod 700 *
./makefile

counter=0
for i in `seq 1 5`;do
	echo Running test No. $i
	while (( $counter < 100))
	do
		echo Iteration No. $counter
		./test$i
		let counter++
	done
let counter=0
done
