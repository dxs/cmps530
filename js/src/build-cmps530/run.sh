#!/bin/bash

for i in $(ls sunspider/*.js); do
    echo "<=== $i ===>"
    ./js $i
done
