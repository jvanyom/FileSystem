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
run "./escribir disco '123456789' 0"
run "./leer_sf disco"
run "time ./truncar disco 1 409605001"
run "./leer_sf disco"
run "time ./truncar disco 1 30725003"
run "./leer_sf disco"
run "time ./truncar disco 1 209008"
run "./leer_sf disco"
run "time ./truncar disco 1 9005"
run "./leer_sf disco"
run "time ./truncar disco 1 0"
run "./leer_sf disco"
