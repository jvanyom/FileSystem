#!/bin/bash

source trun.sh

./structure.sh > /dev/null

trun "../my_rn disco /dir1/dir12/fic122 fic121"
trun "../my_cat disco /dir2/dir22/fic221"
trun "../my_rn disco /dir2/dir22/fic221 fic222"
trun "../my_cat disco /dir2/dir22/fic221"
trun "../my_cat disco /dir2/dir22/fic222"
trun "../my_ls -l disco /dir2/dir22/"
trun "../my_cat disco /dir1/dir12/fic122"
trun "../my_rn disco /dir1/dir12/fic122 fic123"
trun "../my_cat disco /dir1/dir12/fic122"
trun "../my_cat disco /dir1/dir12/fic123"
trun "../my_ls -l disco /dir1/dir12/"
trun "../my_rn disco /dir2/dir23/ dir24"
trun "../my_ls -l disco /dir2/"

trun "./leer_sf disco"
trun "../my_rn disco /dir1/dir12/fic123 fic122"
trun "../my_rn disco /dir2/dir22/fic222 fic221"
trun "../my_rn disco /dir2/dir24/ dir23"
trun "../my_ls -l disco /dir1/dir12/"
trun "../my_ls -l disco /dir2/dir22/"
trun "../my_ls -l disco /dir2/"

trun "./leer_sf disco"