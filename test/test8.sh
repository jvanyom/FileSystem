#!/bin/bash

source trun.sh

clear
make clean
make

trun "../my_mkfs disco 100000"

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
trun "../my_ls disco /dir1/dir11/"
trun "../my_ls -l disco /dir1/dir11/"
trun "../my_ls -l disco /dir1/dir12/"
trun "../my_touch disco 6 /dir1/dir11/fic111"
trun "../my_mkdir disco 6 /dir1/dir11/fic111/dir12/"
trun "../my_touch disco 6 /dir1/dir11/dir12/fic111"
trun "../my_mkdir disco 9 /dir2/"
trun "../my_ls -l disco /dir1/dir11/fic111"