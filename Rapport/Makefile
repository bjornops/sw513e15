TEX:=$(shell ls *.tex)
OTHER = *~ *.aux *.dvi *.toc *.bbl *.blg *.gz *.out *.thm *.ps *.idx *.ilg *.ind *.tdo *.cb *.cb2 *.lot *.soc *

pdflatex: SW513E15.tex
	pdflatex --synctex=1 SW513E15.tex
	bibtex SW513E15
	pdflatex --synctex=1 SW513E15.tex
	pdflatex --synctex=1 SW513E15.tex
	pdflatex --synctex=1 SW513E15.tex
clean:
	git clean -x -f -e SW513E15.pdf

fast: SW513E15.tex
	pdflatex --synctex=1 -interaction batchmode SW513E15.tex
	bibtex SW513E15
	pdflatex --synctex=1 -interaction batchmode SW513E15.tex
	pdflatex --synctex=1 -interaction batchmode SW513E15.tex
	pdflatex --synctex=1 -interaction batchmode SW513E15.tex