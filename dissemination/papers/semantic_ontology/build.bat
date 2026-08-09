@echo off
REM Build "The Semantic Ontology of Actualization".
REM Bibliography is an inline thebibliography, so NO biber/bibtex pass is
REM needed -- two pdflatex passes resolve all refs and the TikZ layout.
REM Figures: run  python figures\make_figures.py  first (fig2..fig7);
REM fig1 and fig8 are TikZ and compile in-document.
cd /d "%~dp0"
pdflatex -interaction=nonstopmode -halt-on-error -file-line-error semantic_ontology.tex
pdflatex -interaction=nonstopmode -halt-on-error -file-line-error semantic_ontology.tex
echo.
echo === build complete ===
