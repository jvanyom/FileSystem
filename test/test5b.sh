#!/bin/bash

source trun.sh

clear
make clean
make

trun "../my_mkfs disco 100000"

trun "./escribir disco '$(cat text.txt)' 1"
trun "./leer disco 2 > ext4.txt"
trun "ls -l ext4.txt"
trun "cat ext4.txt"
trun "./permitir"
trun "./permitir disco 2 0"
trun "./leer disco 2"