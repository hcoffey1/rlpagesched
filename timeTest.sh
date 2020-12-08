#!/bin/bash

if [ ! -d "time" ]
then
mkdir time
fi

for i in {1..32}
do
    echo ${i}
    sed -i "s/Page Select: .*/Page Select: ${i}/g" ./config.txt 
    $(time ./build/pagesim -c ./config.txt -s 2 -t ./medium.log -e 1000)  > ./time/e_1000_ps_${i}.log 2>&1
done