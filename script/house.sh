#!/bin/bash
rm -f *.out
pkill house

for i in $(seq $1 $2)
do
    ./bin/house $i $3 $4 > /dev/null 2> priv/$i.out &
done
