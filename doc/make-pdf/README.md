To make the PDF, I run `wkhtmltox-0.12.6-1` on Windows 10 (the
`wkhtmltox-0.12.6-1.mxe-cross-win64` release) against the file
using the shell script (run in Cygwin) `make.manual.sh`.  I have
tried running `wkhtmltox` in Liunx, but the Linux version has a bug
where it uses system fonts for the PDF, ignoring the fonts specified
in the CSS file (yes, I ran `fc-cache -fv`, and, yes, my fonts were
seen, but, no, `wkhtmltox` still would not use my fonts).

I use the fonts over at [my font 
repo](https://github.com/samboy/CaulixtlaFonts) to render this PDF file;
look in the folders `DesktopFonts/` and `DesktopFonts/ExtraSymbols/`.

Note that `wkhtmltox` has a bug where it will silently put symbols
from system fonts in to PDF files if it can not find a symbol in a
user-specified font.  The way I make sure this is not happening is
by expanding the fonts I use to render the PDF to have all of the 
missing symbols (§, –, ꞏ, ≤/⩽), then open up the PDF document in 
Fontforge to make sure all embedded symbols come from open-source 
fonts.
