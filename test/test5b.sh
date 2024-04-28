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

run "./escribir disco '$(cat text.txt)' 1"
run "./leer disco 2 > ext4.txt"
run "ls -l ext4.txt"
run "cat ext4.txt"
run "./permitir"
run "./permitir disco 2 0"
run "./leer disco 2"