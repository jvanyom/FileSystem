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

print "./escribir disco '$(cat text.txt)' 0"
./escribir disco "$(cat text.txt)" 0

print "./permitir disco 1 0"
./permitir disco 1 0

print "./leer disco 1"
./leer disco 1

print "./permitir disco 1 6"
./permitir disco 1 6

print "./leer disco 1 > ext1.txt"
./leer disco 1 > ext1.txt

print "ls -l ext1.txt"
ls -l ext1.txt

print "./leer_sf disco"
./leer_sf disco

print "./truncar disco 1 409605001"
./truncar disco 1 409605001

print "./leer_sf disco"
./leer_sf disco

print "./truncar disco 1 30725003"
./truncar disco 1 30725003

print "./leer_sf disco"
./leer_sf disco

print "./truncar disco 1 209008"
./truncar disco 1 209008

print "./leer_sf disco"
./leer_sf disco

print "./truncar disco 1 9005"
./truncar disco 1 9005

print "./leer disco 1 > ext1.txt"
./leer disco 1 > ext1.txt

print "ls -l ext1.txt"
ls -l ext1.txt

print "cat ext1.txt"
cat ext1.txt

print "./leer_sf disco"
./leer_sf disco

print "./truncar disco 1 0"
./truncar disco 1 0

print "./leer_sf disco"
./leer_sf disco