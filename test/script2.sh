#!/bin/bash

source trun.sh

clear
make clean
make

trun "../my_mkfs disco 100000"

trun "./leer_sf disco"

trun "../my_mkdir"
trun "../my_mkdir disco 7 /"
trun "../my_mkdir disco 6 dir1/"
trun "../my_mkdir disco 6 /dir1/"
trun "../my_mkdir disco 6 /dir1/dir11/"

trun "../my_chmod"
trun "../my_chmod disco 1 /dir1/dir11/"

trun "../my_touch disco 6 /dir1/dir11/fic111"
trun "../my_chmod disco 2 /dir1/dir11/"

trun "../my_touch disco 6 /dir1/dir11/fic111"
trun "../my_chmod disco 6 /dir1/dir11/"
trun "../my_touch disco 6 /dir1/dir11/fic111"
trun "../my_touch disco 6 /dir1/dir11/fic112"

trun "../my_ls -l disco /"
trun "../my_stat disco /dir1/"
trun "../my_ls -l disco /dir1/"
trun "../my_stat disco /dir1/dir11/"
trun "../my_ls -l disco /dir1/dir11/"
trun "../my_ls -l disco /dir1/dir12/"

trun "../my_touch disco 6 /dir1/dir11/fic111"
trun "../my_mkdir disco 6 /dir1/dir11/fic111/dir12/"
trun "../my_touch disco 6 /dir1/dir11/dir12/fic111"
trun "../my_mkdir disco 9 /dir2/"

trun "./leer_sf disco"
trun "../my_write disco /dir1/dir11/fic111 '$(cat text.txt)' 0"

trun "../my_ls -l disco /dir1/dir11/fic111"
trun "./leer_sf disco"
trun "../my_cat disco /dir1/dir11/fic111 > ext1.txt"

trun "ls -l ext1.txt"
trun "../my_chmod disco 4 /dir1/dir11/fic111"

trun "../my_write disco /dir1/dir11/fic111 'lo que sea' 209000"
trun "../my_ls -l disco /dir1/dir11/"

trun "../my_write disco /dir1/dir11/fic112 'hola1' 209000"
trun "../my_stat disco /dir1/dir11/fic112"
trun "sleep 1"

trun "../my_write disco /dir1/dir11/fic112 'hola2' 9000"
trun "../my_stat disco /dir1/dir11/fic112"
trun "sleep 1"

trun "../my_write disco /dir1/dir11/fic112 'hola3' 9100"
trun "../my_stat disco /dir1/dir11/fic112"
trun "sleep 1"

trun "../my_write disco /dir1/dir11/fic112 'hola4' 275000"
trun "../my_stat disco /dir1/dir11/fic112"

trun "../my_cat disco /dir1/dir11/fic112"
trun "../my_ls -l disco /dir1/dir11/"

trun "../my_touch disco 6 /dir1/dir11/fic113"
trun "../my_write disco /dir1/dir11/fic113 'hellooooooo' 409605000"

trun "../my_mkdir disco 6 /dir2/"
trun "../my_mkdir disco 6 /dir2/dir21/"

trun "../my_link disco /dir1/dir11/fic113 /dir2/dir21/fic211"

trun "../my_cat disco /dir2/dir21/fic211"
trun "../my_stat disco /dir1/dir11/fic113"
trun "../my_stat disco /dir2/dir21/fic211"

trun "../my_link disco /dir1/dir11/fic14 /di2/dir21/fic212"
trun "../my_link disco /dir1/dir11/fic113 /dir2/dir21/fic211"

trun "../my_rmdir disco /dir2/dir21/"
trun "../my_rm disco /dir2/dir21/fic211"

trun "../my_stat disco /dir1/dir11/fic113"
trun "./leer_sf disco"

trun "../my_rm disco /dir1/dir11/fic113"
trun "./leer_sf disco"

trun "../my_rm disco /dir2/dir21/fic211"
trun "../my_rmdir disco /dir2/dir21/"
trun "../my_ls -l disco /dir2/"
trun "../my_mkdir disco 6 /dir3/"

for i in $(seq 0 16); do
  trun "../my_mkdir disco 6 /dir3/sd$i/"
done

trun "../my_stat disco /dir3/"
trun "../my_ls disco /dir3/"

trun "../my_rmdir disco /dir3/sd3/"

trun "../my_stat disco /dir3/"
trun "../my_ls -l disco /dir3/"

make clean > /dev/null