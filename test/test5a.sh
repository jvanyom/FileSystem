#!/bin/bash

source trun.sh

clear
make clean
make

trun "../my_mkfs disco 100000"

trun "./leer_sf disco"
trun "./escribir"
trun "./escribir disco '123456789' 0"
trun "./leer disco 1 > ext1.txt"
trun "ls -l ext1.txt"
trun "./escribir disco '123456789' 1"
trun "./leer disco 2 > ext2.txt"
trun "ls -l ext2.txt"
trun "cat ext2.txt"
trun "./leer disco 2"
trun "./leer disco 5 > ext3.txt"
trun "ls -l ext3.txt"
trun "cat ext3.txt"
trun "./leer disco 5"