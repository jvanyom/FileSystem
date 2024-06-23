#!/bin/bash

source trun.sh

clear
make clean
make

trun "../my_mkfs disco 100000"

trun "../my_touch disco 6 /fichero"
trun "../my_ls -l  disco /"
trun "../my_write  disco /fichero 'hola que tal' 5120"
trun "../my_ls -l  disco /"
trun "./leer_sf disco"
trun "../my_chmod disco 4 /fichero"
trun "../my_write  disco /fichero 'estoy estupendamente' 256000"
trun "../my_ls -l  disco /fichero"
trun "../my_mkdir disco 6 /dir1/"
trun "../my_touch disco 6 /dir1/fic1"

trun "../my_write disco /dir1/fic1 hola1  256000"
trun "../my_stat disco /dir1/fic1"
trun "sleep 1"

trun "../my_write disco /dir1/fic1 hola2  5120"
trun "../my_stat disco /dir1/fic1"
trun "sleep 1"

trun "../my_write disco /dir1/fic1 hola3  5200"
trun "../my_stat disco /dir1/fic1"
trun "sleep 1"

trun "../my_write disco /dir1/fic1 hola4 256010"
trun "../my_stat disco /dir1/fic1"

trun "../my_touch disco 6 /dir1/fic2"
trun "../my_write disco /dir1/fic2 '$(cat text.txt)' 1000"
trun "../my_cat disco /dir1/fic2"
trun "../my_write disco /dir1/fic2 '******************************' 10000"
trun "../my_ls -l  disco /dir1/"
trun "../my_cat disco /dir1/fic2"
trun "../my_touch disco 6 /dir1/fic3"

trun "./batch_write disco /dir1/fic3 '--texto repetido en 10 bloques--' 0"
trun "../my_stat disco /dir1/fic3"
trun "../my_cat disco /dir1/fic3"