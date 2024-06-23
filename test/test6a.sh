#!/bin/bash

source trun.sh

clear
make clean
make

trun "../my_mkfs disco 100000"

trun "./leer_sf disco"
trun "./escribir disco '123456789' 0"
trun "./leer_sf disco"
trun "time ./truncar disco 1 0"
trun "./leer_sf disco"