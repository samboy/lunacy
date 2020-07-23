#!/bin/sh

if [ ! -e /mingw ] ; then
	echo Only run this in Mingw
	exit 1
fi

# No symlinks
rm -f lauxlib.h lua.h luaconf.h lualib.h

for a in lauxlib.h lua.h luaconf.h lualib.h ; do
	cp ../src/$a .
done

cd ..
make
cd mmLunacyDNS
gcc -Os -DMINGW -c -o mmLunacyDNS.o mmLunacyDNS.c
gcc -Os -o mmLunacyDNS mmLunacyDNS.o ../src/liblua.a -lm -lwsock32
