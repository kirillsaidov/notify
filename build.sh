#!/usr/bin/env bash

mkdir -p bin
gcc -o bin/notify notify.c

echo "Built notify.c"
echo "Saved to 'bin/notify'"