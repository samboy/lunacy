#!/bin/sh

# Clean up
rm -f *.o *.exe mmLunacyDNS

# Win32 build env has no symlinks
rm -f lauxlib.h lua.h luaconf.h lualib.h
for a in lauxlib.h lua.h luaconf.h lualib.h ; do
        ln -s ../src/$a $a
done

