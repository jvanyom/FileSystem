#!/bin/bash

source trun.sh

clear
make clean
make

trun "../my_mkfs disco 100000"

trun "../my_mkdir disco 6 /dir1/"
trun "../my_mkdir disco 6 /dir1/dir11/"
trun "../my_touch disco 6 /dir1/dir11/fic1"
trun "../my_write disco /dir1/dir11/fic1 'hellooooooo' 0"
trun "../my_mkdir disco 6 /dir2/"
trun "../my_mkdir disco 6 /dir2/dir21/"
trun "../my_link disco /dir1/dir11/fic1 /dir2/dir21/fic2"
trun "../my_cat disco /dir2/dir21/fic2"
trun "../my_stat disco /dir1/dir11/fic1"
trun "../my_stat disco /dir2/dir21/fic2"
trun "../my_link disco /dir1/dir11/fic3 /di2/dir21/fic4"
trun "../my_touch disco 6 /dir1/dir11/fic3"
trun "../my_link disco /dir1/dir11/fic3 /dir2/dir21/fic4"
trun "../my_link disco /dir1/dir11/fic3 /dir2/dir21/fic5"
trun "../my_stat disco /dir1/dir11/fic3"
trun "../my_link disco /dir1/dir11/fic3 /dir2/dir21/fic2"

trun "../my_rmdir disco /dir2/dir21/"
trun "../my_rm disco /dir2/dir21/fic2"
trun "../my_stat disco /dir1/dir11/fic1"
trun "../my_rm disco /dir2/dir21/fic2"
trun "../my_rmdir disco /dir2/dir21/"
trun "../my_ls -l disco /dir2/dir21/"
trun "../my_rm disco /dir2/dir21/fic4"
trun "../my_rm disco /dir2/dir21/fic5"
trun "../my_rmdir disco /dir2/dir21/"
trun "../my_ls -l disco /dir2/"

trun "../my_mkdir disco 6 /d1/"

for i in $(seq 0 16); do
    trun "../my_mkdir disco 6 /d1/sd$i/"
done

trun "../my_stat disco /d1/"
trun "../my_ls disco /d1/"
trun "../my_rmdir disco /d1/sd3/"
trun "../my_stat disco /d1/"
trun "../my_ls -l disco /d1/"