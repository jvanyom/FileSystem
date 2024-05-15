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

run "../my_mkdir disco 6 /dir1/"
run "../my_mkdir disco 6 /dir1/dir11/"
run "../my_touch disco 6 /dir1/dir11/fic1"
run "../my_write disco /dir1/dir11/fic1 'hellooooooo' 0"
run "../my_mkdir disco 6 /dir2/"
run "../my_mkdir disco 6 /dir2/dir21/"
run "../my_link disco /dir1/dir11/fic1 /dir2/dir21/fic2"
run "../my_cat disco /dir2/dir21/fic2"
run "../my_stat disco /dir1/dir11/fic1"
run "../my_stat disco /dir2/dir21/fic2"
run "../my_link disco /dir1/dir11/fic3 /di2/dir21/fic4"
run "../my_touch disco 6 /dir1/dir11/fic3"
run "../my_link disco /dir1/dir11/fic3 /dir2/dir21/fic4"
run "../my_link disco /dir1/dir11/fic3 /dir2/dir21/fic5"
run "../my_stat disco /dir1/dir11/fic3"
run "../my_link disco /dir1/dir11/fic3 /dir2/dir21/fic2"

run "../my_rmdir disco /dir2/dir21/"
run "../my_rm disco /dir2/dir21/fic2"
run "../my_stat disco /dir1/dir11/fic1"
run "../my_rm disco /dir2/dir21/fic2"
run "../my_rmdir disco /dir2/dir21/"
run "../my_ls -l disco /dir2/dir21/"
run "../my_rm disco /dir2/dir21/fic4"
run "../my_rm disco /dir2/dir21/fic5"
run "../my_rmdir disco /dir2/dir21/"
run "../my_ls -l disco /dir2/"

run "../my_mkdir disco 6 /d1/"

for i in $(seq 0 16); do
    run "../my_mkdir disco 6 /d1/sd$i/"
done

run "../my_stat disco /d1/"
run "../my_ls disco /d1/"
run "../my_rmdir disco /d1/sd3/"
run "../my_stat disco /d1/"
run "../my_ls -l disco /d1/"