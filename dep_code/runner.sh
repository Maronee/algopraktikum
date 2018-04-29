#!/usr/bin/bash 


# $1 ass1; $2 task1
cd algopraktikum; mpicc -o app ./$1/mcz/$2/hot.c && mpiexec -f ./$1/mcz/$2/hosts  -n 9 ./$1/mcz/$2/app
cd ..
