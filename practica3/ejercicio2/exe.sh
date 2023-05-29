#!/bin/bash
n=10
./cocinero & 
echo "cocinero $!"
sleep 5 
for i in `seq 1 1 $n`
do
./salvajes $i & 
echo "salvaje $i pid=$!"
done
wait