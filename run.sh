#!/bin/bash

set -xe
gcc -Wall -Wextra -ggdb -o main main.c
./main
