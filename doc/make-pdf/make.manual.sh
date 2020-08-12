./wkhtmltopdf -s Letter --footer-center '[Page]' \
	--footer-font-name Caulixtla009Serif --minimum-font-size 19 \
	toc --xsl-style-sheet lunacy-toc.xsl \
	lunacy-manual.html lunacy-manual.pdf
