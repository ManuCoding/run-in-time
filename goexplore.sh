#!/bin/bash

set -xe
gcc -ggdb -o cexplore cexplore.c
false
as -o explore.o explore.asm
ld -o explore explore.o
hexdump -s 4096 -n 256 -C explore
