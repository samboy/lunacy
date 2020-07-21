#!/bin/sh

# With only one file, I will not bother with a Makefile

cd ..
make 
cd mmLunacyDNS
gcc -c -o mmLunacyDNS.o mmLunacyDNS.c
gcc -o mmLunacyDNS mmLunacyDNS.o ../src/liblua.a -lm
