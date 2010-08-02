#!/bin/bash

set -o nounset
set -o errexit

PREFIX=${PREFIX:-}
VERIFY=${VERIFY:-yes}
EMU_HACK=${EMU_HACK:-yes}


DASHDASH=""
if [[ "${PREFIX}" =~ sel_ldr ]] ; then
  DASHDASH="--"
fi

rm -f  mesa.log mesa.ppm mesa.in numbers

ln -s  data/ref/input/* .

${PREFIX} $1 ${DASHDASH} -frames 1000 -meshfile mesa.in -ppmfile mesa.ppm

if [[ "${VERIFY}" != "no" ]] ; then
  echo "VERIFY"
  cmp  mesa.log  data/ref/output/mesa.log
  ../specdiff.sh -a 6 -l 10 mesa.ppm data/ref/output/mesa.ppm
fi


echo "OK"
