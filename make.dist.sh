#!/bin/sh

rm -fr lunacy-dist/
mkdir lunacy-dist/
cp bin/lunacy.exe lunacy-dist/
cp doc/lunacy-manual.html lunacy-dist/
mkdir lunacy-dist/examples/
cp test/*lua lunacy-dist/examples
mkdir lunacy-dist/LUAstuff
cp LUAstuff/* lunacy-dist/LUAstuff/
cp COPYING lunacy-dist/COPYING.txt
cp README.md lunacy-dist/README.txt
unix2dos lunacy-dist/COPYING.txt
unix2dos lunacy-dist/README.txt
mv lunacy-dist/LUAstuff/COPYING.md lunacy-dist/LUAstuff/COPYING.txt
unix2dos lunacy-dist/LUAstuff/COPYING.txt
