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

run "../my_touch disco 6 /fichero"
run "../my_ls -l  disco /"
run "../my_write  disco /fichero 'hola que tal' 5120"
run "../my_ls -l  disco /"
run "./leer_sf disco"
run "../my_chmod disco 4 /fichero"
run "../my_write  disco /fichero 'estoy estupendamente' 256000"
run "../my_ls -l  disco /fichero"
run "../my_mkdir disco 6 /dir1/"
run "../my_touch disco 6 /dir1/fic1"

run "../my_write disco /dir1/fic1 hola1  256000"
run "../my_stat disco /dir1/fic1"
run "sleep 1"

run "../my_write disco /dir1/fic1 hola2  5120"
run "../my_stat disco /dir1/fic1"
run "sleep 1"

run "../my_write disco /dir1/fic1 hola3  5200"
run "../my_stat disco /dir1/fic1"
run "sleep 1"

run "../my_write disco /dir1/fic1 hola4 256010"
run "../my_stat disco /dir1/fic1"

run "../my_touch disco 6 /dir1/fic2"
run "../my_write disco /dir1/fic2 '$(cat text.txt)' 1000"
run "../my_cat disco /dir1/fic2"
run "../my_write disco /dir1/fic2 '******************************' 10000"
run "../my_ls -l  disco /dir1/"
run "../my_cat disco /dir1/fic2"
run "../my_touch disco 6 /dir1/fic3"

run "./batch_write disco /dir1/fic3 '--texto repetido en 10 bloques--' 0"
run "../my_stat disco /dir1/fic3"
run "../my_cat disco /dir1/fic3"