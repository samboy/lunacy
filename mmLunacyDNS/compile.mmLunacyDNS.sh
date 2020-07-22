#!/bin/sh

# With only one file, I will not bother with a Makefile

cd ..
make 
cd mmLunacyDNS
gcc -Os -c -o mmLunacyDNS.o mmLunacyDNS.c
gcc -Os -o mmLunacyDNS mmLunacyDNS.o ../src/liblua.a -lm
