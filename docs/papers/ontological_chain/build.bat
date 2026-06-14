@echo off
REM Build the FTD Ontological Chain PDF (latexmk unavailable -- no Perl).
cd /d %~dp0
pdflatex -interaction=nonstopmode ontological_chain.tex
biber ontological_chain
pdflatex -interaction=nonstopmode ontological_chain.tex
pdflatex -interaction=nonstopmode ontological_chain.tex
echo Built ontological_chain.pdf
