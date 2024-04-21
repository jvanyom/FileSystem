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

print "./escribir disco '$(cat text.txt)' 1"
./escribir disco "$(cat text.txt)" 1

print "./leer disco 2 > ext4.txt"
./leer disco 2 > ext4.txt

print "ls -l ext4.txt"
ls -l ext4.txt

print "cat ext4.txt"
cat ext4.txt

print "./permitir"
./permitir

print "./permitir disco 2 0"
./permitir disco 2 0

print "./leer disco 2"
./leer disco 2