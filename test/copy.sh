#!/bin/bash

source trun.sh

./structure.sh > /dev/null

trun "../my_cp disco /dir1/dir12/fic121 /dir3/"
trun "./leer_sf disco"

trun "../my_stat disco /dir1/dir12/fic121"
trun "../my_stat disco /dir3/fic121"

trun "../my_cat disco /dir1/dir12/fic121"
trun "../my_cat disco /dir3/fic121"

trun "../my_cp disco /dir2/ /dir3/dir32/"
trun "./leer_sf disco"

trun "../my_ls -l disco /dir2/"
trun "../my_ls -l disco /dir3/dir32/dir2/"

trun "../my_ls -l disco /dir2/dir21/"
trun "../my_ls -l disco /dir3/dir32/dir2/dir21/"

trun "../my_ls -l disco /dir2/dir22/"
trun "../my_ls -l disco /dir3/dir32/dir2/dir22/"

trun "../my_stat disco /dir2/dir21/fic211"
trun "../my_stat disco /dir3/dir32/dir2/dir21/fic211"

trun "../my_stat disco /dir2/dir22/fic221"
trun "../my_stat disco /dir3/dir32/dir2/dir22/fic221"

trun "../my_cat disco /dir2/dir21/fic211"
trun "../my_cat disco /dir3/dir32/dir2/dir21/fic211"

trun "../my_cat disco /dir2/dir22/fic221"
trun "../my_cat disco /dir3/dir32/dir2/dir22/fic221"