#!/bin/bash

source trun.sh

clear
make clean
make

trun "../my_mkfs disco 100000"

trun "../my_mkdir disco 6 /dir1/"
trun "../my_mkdir disco 6 /dir1/dir11/"
trun "../my_mkdir disco 6 /dir1/dir12/"
trun "../my_touch disco 6 /dir1/dir12/fic121"
trun "../my_touch disco 6 /dir1/dir12/fic122"
trun "../my_mkdir disco 6 /dir1/dir13/"
trun "../my_touch disco 6 /dir1/fic11"
trun "../my_mkdir disco 6 /dir2/"
trun "../my_mkdir disco 6 /dir2/dir21/"
trun "../my_touch disco 6 /dir2/dir21/fic211"
trun "../my_mkdir disco 6 /dir2/dir22/"
trun "../my_touch disco 6 /dir2/dir22/fic221"
trun "../my_mkdir disco 6 /dir2/dir23/"
trun "../my_mkdir disco 6 /dir3/"
trun "../my_touch disco 6 /dir3/fic31"
trun "../my_mkdir disco 6 /dir3/dir31/"
trun "../my_mkdir disco 6 /dir3/dir32/"
trun "../my_mkdir disco 6 /dir3/dir33/"

trun "../my_write disco /dir1/dir12/fic121 'Haz un buen día - ' 0"
trun "../my_write disco /dir1/dir12/fic121 'Hoy puede ser un gran día, plantéatelo así. Aprovecharlo o que pase de largo depende en parte de ti. - ' 5120"
trun "../my_write disco /dir1/dir12/fic121 'Si la rutina te aplasta dile que ya basta de mediocridad - ' 48000"
trun "../my_write disco /dir1/dir12/fic122 'Pelea por lo que quieres y no desesperes si algo no anda bien - ' 90000"
trun "../my_write disco /dir1/dir12/fic122 'Hoy puede ser un gran día y mañana también - ' 15000"
trun "../my_write disco /dir1/fic11 'Hoy puede ser un gran día, date una oportunidad - ' 70000"
trun "../my_write disco /dir2/dir21/fic211 'Hoy puede ser un gran día, imposible de recuperar - ' 100"
trun "../my_write disco /dir2/dir22/fic221 'Hoy puede ser un gran día, duro con él - ' 3333"
trun "../my_write disco /dir3/fic31 'Que la fuerza te acompañe - ' 1000"

trun "../my_ls -l disco /"
trun "../my_ls -l disco /dir1/"
trun "../my_cat disco /dir1/fic11"
trun "../my_stat disco /dir1/fic11"
trun "../my_ls -l disco /dir1/dir12/"
trun "../my_cat disco /dir1/dir12/fic121"
trun "../my_stat disco /dir1/dir12/fic121"
trun "../my_cat disco /dir1/dir12/fic122"
trun "../my_stat disco /dir1/dir12/fic122"
trun "../my_ls -l disco /dir2/"
trun "../my_ls -l disco /dir2/dir21/"
trun "../my_cat disco /dir2/dir21/fic211"
trun "../my_stat disco /dir2/dir21/fic211"
trun "../my_ls -l disco /dir2/dir22/"
trun "../my_cat disco /dir2/dir22/fic221"
trun "../my_stat disco /dir2/dir22/fic221"
trun "../my_ls -l disco /dir3/"
trun "../my_cat disco /dir3/fic31"
trun "../my_stat disco /dir3/fic31"

trun "./leer_sf disco"