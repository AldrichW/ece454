#!/bin/bash

make clean
make
if [ "$2" == v ]; then
    ./gol_verify 1000 inputs/1k.pbm outputs/1k.pbm
fi
/usr/bin/time -f "%e real" ./gol "$1" inputs/1k.pbm outputs/1k.pbm
