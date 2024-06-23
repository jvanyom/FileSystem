#!/bin/bash

source trun.sh

./structure.sh > /dev/null

trun "../my_rm -r disco /dir3/"
trun "./leer_sf disco"

trun "../my_rm -r disco /dir2/"
trun "./leer_sf disco"

trun "../my_rm -r disco /dir1/"
trun "./leer_sf disco"