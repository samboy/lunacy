#!/bin/sh

rm -fr lunacy-dist/
mkdir lunacy-dist/
cp bin/lunacy.exe lunacy-dist/
cp doc/lunacy-manual.html lunacy-dist/
cp doc/*css lunacy-dist/
mkdir lunacy-dist/examples/
cp test/*lua lunacy-dist/examples
mkdir lunacy-dist/LUAstuff
cp LUAstuff/* lunacy-dist/LUAstuff/
rm lunacy-dist/LUAstuff/rg32*
rm lunacy-dist/LUAstuff/bit32.lua
rm lunacy-dist/LUAstuff/Lua51calc.lua
cp COPYING lunacy-dist/COPYING.txt
cp COPYING.spawner lunacy-dist/COPYING.spawner.txt
cp README.md lunacy-dist/README.txt
unix2dos lunacy-dist/COPYING.txt
unix2dos lunacy-dist/COPYING.spawner.txt
unix2dos lunacy-dist/README.txt
mv lunacy-dist/LUAstuff/COPYING.md lunacy-dist/LUAstuff/COPYING.txt
unix2dos lunacy-dist/LUAstuff/COPYING.txt
