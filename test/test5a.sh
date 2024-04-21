#!/bin/bash

LIGHT_GREEN="\x1B[38;2;17;245;120m$"
RESET="\x1b[0m"

print() {
  echo -e "\n################################################################################"
  echo -e "$LIGHT_GREEN $1 $RESET"
}

clear
make clean
make

print "./mi_mkfs disco 100000"
./mi_mkfs disco 100000

print "./leer_sf disco"
./leer_sf disco

print "./escribir"
./escribir

print "./escribir disco '123456789' 0"
./escribir disco "123456789" 0

print "./leer disco 1 > ext1.txt"
./leer disco 1 > ext1.txt

print "ls -l ext1.txt"
ls -l ext1.txt

print "./escribir disco '123456789' 1"
./escribir disco "123456789" 1

print "./leer disco 2 > ext2.txt"
./leer disco 2 > ext2.txt

print "ls -l ext2.txt"
ls -l ext2.txt

print "cat ext2.txt"
cat ext2.txt

print "./leer disco 2"
./leer disco 2

print "./leer disco 5 > ext3.txt"
./leer disco 5 > ext3.txt

print "ls -l ext3.txt"
ls -l ext3.txt

print "cat ext3.txt"
cat ext3.txt

print "./leer disco 5"
./leer disco 5