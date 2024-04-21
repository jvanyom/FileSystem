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

print "./escribir disco '$(cat text.txt)' 0"
./escribir disco "$(cat text.txt)" 0

print "time ./truncar disco 1 0"
time ./truncar disco 1 0