#!/bin/bash

make $1
rm -rf $4
(/usr/bin/time ./$1 $2 $3) 2>> $4 
(/usr/bin/time ./$1 $2 $3) 2>> $4 
(/usr/bin/time ./$1 $2 $3) 2>> $4 
(/usr/bin/time ./$1 $2 $3) 2>> $4 
(/usr/bin/time ./$1 $2 $3) 2>> $4 
