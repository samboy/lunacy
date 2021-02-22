#!/bin/sh
ebook-convert lunacy-manual.html lunacy-manual.mobi \
	--mobi-file-type=both --change-justification=left 
ebook-convert lunacy-manual.html lunacy-manual.epub
lynx -dump -nolist lunacy-manual.html > lunacy-manual.txt
