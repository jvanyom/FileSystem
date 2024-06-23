#!/bin/bash

source trun.sh

clear
make clean
make

trun "../my_mkfs disco 100000"

trun "./leer_sf disco"
trun "./escribir"
trun "./escribir disco '$(cat text.txt)' 0"
trun "./permitir disco 1 0"
trun "./leer disco 1"
trun "./permitir disco 1 6"
trun "./leer disco 1 > ext1.txt"
trun "ls -l ext1.txt"
trun "./leer_sf disco"
trun "./truncar disco 1 409605001"
trun "./leer_sf disco"
trun "./truncar disco 1 30725003"
trun "./leer_sf disco"
trun "./truncar disco 1 209008"
trun "./leer_sf disco"
trun "./truncar disco 1 9005"
trun "./leer disco 1 > ext1.txt"
trun "ls -l ext1.txt"
trun "cat ext1.txt"
trun "./leer_sf disco"
trun "./truncar disco 1 0"
trun "./leer_sf disco"