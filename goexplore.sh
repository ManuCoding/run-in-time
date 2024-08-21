#!/bin/bash

set -xe

as -o explore.o explore.asm
gcc -o cexplore.o -c cexplore.c
gcc -o explore cexplore.o explore.o
#hexdump -s 4096 -n 256 -C explore
#objdump -D explore
./explore
