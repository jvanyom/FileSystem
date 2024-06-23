#!/bin/bash

source trun.sh

clear
make clean
make

trun "../my_mkfs disco 100000"

trun "./leer_sf disco"
trun "./escribir disco '$(cat text.txt)' 0"
trun "./leer_sf disco"

trun "time ./truncar disco 1 409605001"
trun "./leer_sf disco"

trun "time ./truncar disco 1 30725003"
trun "./leer_sf disco"

trun "time ./truncar disco 1 209008"
trun "./leer_sf disco"

trun "time ./truncar disco 1 9005"
trun "./leer_sf disco"

trun "time ./truncar disco 1 0"
trun "./leer_sf disco"