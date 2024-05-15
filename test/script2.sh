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

run "../my_mkdir"
run "../my_mkdir disco 7 /"
run "../my_mkdir disco 6 dir1/"
run "../my_mkdir disco 6 /dir1/"
run "../my_mkdir disco 6 /dir1/dir11/"

run "../my_chmod"
run "../my_chmod disco 1 /dir1/dir11/"

run "../my_touch disco 6 /dir1/dir11/fic111"
run "../my_chmod disco 2 /dir1/dir11/"

run "../my_touch disco 6 /dir1/dir11/fic111"
run "../my_chmod disco 6 /dir1/dir11/"
run "../my_touch disco 6 /dir1/dir11/fic111"
run "../my_touch disco 6 /dir1/dir11/fic112"

run "../my_ls -l disco /"
run "../my_stat disco /dir1/"
run "../my_ls -l disco /dir1/"
run "../my_stat disco /dir1/dir11/"
run "../my_ls -l disco /dir1/dir11/"
run "../my_ls -l disco /dir1/dir12/"

run "../my_touch disco 6 /dir1/dir11/fic111"
run "../my_mkdir disco 6 /dir1/dir11/fic111/dir12/"
run "../my_touch disco 6 /dir1/dir11/dir12/fic111"
run "../my_mkdir disco 9 /dir2/"

run "./leer_sf disco"
run "../my_write disco /dir1/dir11/fic111 '$(cat text.txt)' 0"

run "../my_ls -l disco /dir1/dir11/fic111"
run "./leer_sf disco"
run "../my_cat disco /dir1/dir11/fic111 > ext1.txt"

run "ls -l ext1.txt"
run "../my_chmod disco 4 /dir1/dir11/fic111"

run "../my_write disco /dir1/dir11/fic111 'lo que sea' 209000"
run "../my_ls -l disco /dir1/dir11/"

run "../my_write disco /dir1/dir11/fic112 'hola1' 209000"
run "../my_stat disco /dir1/dir11/fic112"
run "sleep 1"

run "../my_write disco /dir1/dir11/fic112 'hola2' 9000"
run "../my_stat disco /dir1/dir11/fic112"
run "sleep 1"

run "../my_write disco /dir1/dir11/fic112 'hola3' 9100"
run "../my_stat disco /dir1/dir11/fic112"
run "sleep 1"

run "../my_write disco /dir1/dir11/fic112 'hola4' 275000"
run "../my_stat disco /dir1/dir11/fic112"

run "../my_cat disco /dir1/dir11/fic112"
run "../my_ls -l disco /dir1/dir11/"

run "../my_touch disco 6 /dir1/dir11/fic113"
run "../my_write disco /dir1/dir11/fic113 'hellooooooo' 409605000"

run "../my_mkdir disco 6 /dir2/"
run "../my_mkdir disco 6 /dir2/dir21/"

run "../my_link disco /dir1/dir11/fic113 /dir2/dir21/fic211"

run "../my_cat disco /dir2/dir21/fic211"
run "../my_stat disco /dir1/dir11/fic113"
run "../my_stat disco /dir2/dir21/fic211"

run "../my_link disco /dir1/dir11/fic14 /di2/dir21/fic212"
run "../my_link disco /dir1/dir11/fic113 /dir2/dir21/fic211"

run "../my_rmdir disco /dir2/dir21/"
run "../my_rm disco /dir2/dir21/fic211"

run "../my_stat disco /dir1/dir11/fic113"
run "./leer_sf disco"

run "../my_rm disco /dir1/dir11/fic113"
run "./leer_sf disco"

run "../my_rm disco /dir2/dir21/fic211"
run "../my_rmdir disco /dir2/dir21/"
run "../my_ls -l disco /dir2/"
run "../my_mkdir disco 6 /dir3/"

for i in $(seq 0 16); do
  run "../my_mkdir disco 6 /dir3/sd$i/"
done

run "../my_stat disco /dir3/"
run "../my_ls disco /dir3/"

run "../my_rmdir disco /dir3/sd3/"

run "../my_stat disco /dir3/"
run "../my_ls -l disco /dir3/"

make clean > /dev/null