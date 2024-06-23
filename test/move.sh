#!/bin/bash

source trun.sh

./structure.sh > /dev/null

trun "./leer_sf disco"

trun "../my_ls -l disco /dir2/dir22/"
trun "../my_mv disco /dir2/dir22/fic221 /dir1/fic11"
trun "../my_ls -l disco /dir2/dir21/"
trun "../my_mv disco /dir2/dir22/fic221 /dir2/dir21/"
trun "../my_ls -l disco /dir2/dir22/"
trun "../my_ls -l disco /dir2/dir21/"

trun "./leer_sf disco"

trun "../my_ls -l disco /"
trun "../my_ls -l disco /dir1/"
trun "../my_mv disco /dir3/ /dir1/"
trun "../my_ls -l disco /"
trun "../my_ls -l disco /dir1/"

trun "./leer_sf disco"

trun "../my_mv disco /dir1/ /dir3/"

trun "../my_mv disco /dir2/dir21/fic221 /dir2/dir22/"
trun "../my_ls -l disco /dir2/dir22/"
trun "../my_ls -l disco /dir2/dir21/"

trun "./leer_sf disco"

trun "../my_mv disco /dir1/dir3/ /"
trun "../my_ls -l disco /"
trun "../my_ls -l disco /dir1/"

trun "./leer_sf disco"