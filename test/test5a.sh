#!/bin/bash

LIGHT_GREEN="\x1B[38;2;17;245;120m$"
RESET="\x1b[0m"

run() {
  echo -e "\n################################################################################"
  echo -e "$LIGHT_GREEN $1 $RESET"
  eval "$1"
}

clear
make clean
make

run "../my_mkfs disco 100000"

run "./leer_sf disco"
run "./escribir"
run "./escribir disco '123456789' 0"
run "./leer disco 1 > ext1.txt"
run "ls -l ext1.txt"
run "./escribir disco '123456789' 1"
run "./leer disco 2 > ext2.txt"
run "ls -l ext2.txt"
run "cat ext2.txt"
run "./leer disco 2"
run "./leer disco 5 > ext3.txt"
run "ls -l ext3.txt"
run "cat ext3.txt"
run "./leer disco 5"