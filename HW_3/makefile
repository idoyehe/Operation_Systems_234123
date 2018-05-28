#!/bin/bash

counter=1
while (( $counter <= 5))
do
g++ -g  -pthread -std=c++11 Factory.cxx hw3_test$counter.cpp -o test$counter
echo test No. $counter is Ready!!
let counter++
done 

