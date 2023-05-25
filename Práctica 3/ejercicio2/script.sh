#!/bin/bash
n=25
./cocinero & 
echo "cocinero $!"
& sleep 5 
for((i=0, $i<$n, i++))
do
./salvajes $i & echo "salvaje $i pid=$!"
done
wait