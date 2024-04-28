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
run "../my_ls disco /dir1/dir11/"
run "../my_ls -l disco /dir1/dir11/"
run "../my_ls -l disco /dir1/dir12/"
run "../my_touch disco 6 /dir1/dir11/fic111"
run "../my_mkdir disco 6 /dir1/dir11/fic111/dir12/"
run "../my_touch disco 6 /dir1/dir11/dir12/fic111"
run "../my_mkdir disco 9 /dir2/"
run "../my_ls -l disco /dir1/dir11/fic111"