@echo off
REM Build "The Semantic Ontology of Actualization".
REM Bibliography is an inline thebibliography, so NO biber/bibtex pass is
REM needed -- two pdflatex passes resolve all refs and the TikZ layout.
REM Figures: 14 computed PDFs from two trees, both sharing the style
REM module scripts\experiments\temporal_interior\_figstyle.py --
REM   python figures\make_figures.py                        (6)
REM   python ..\..\..\scripts\experiments\temporal_interior\fig_*.py
REM   python ..\..\..\scripts\experiments\temporal_interior\toy*.py   (8)
REM Every figure is authored at \textwidth = 6.1677 in and included at
REM width=\textwidth, so the scale is 1.0 and rcParams sizes are the
REM printed point sizes.  Never include at a fractional width.
REM The two ontology diagrams are TikZ and compile in-document.
REM Set FTD_FIG_BACKEND=fallback if pdflatex-in-matplotlib is unavailable.
cd /d "%~dp0"
pdflatex -interaction=nonstopmode -halt-on-error -file-line-error semantic_ontology.tex
pdflatex -interaction=nonstopmode -halt-on-error -file-line-error semantic_ontology.tex
echo.
echo === build complete ===
