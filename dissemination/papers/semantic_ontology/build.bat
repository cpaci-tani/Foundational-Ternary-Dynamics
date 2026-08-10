@echo off
REM Build "The Semantic Ontology of Actualization".
REM Bibliography is an inline thebibliography, so NO biber/bibtex pass is
REM needed -- two pdflatex passes resolve all refs and the TikZ layout.
REM Figures: 15 included computed PDFs from two trees, all sharing the style
REM module scripts\experiments\temporal_interior\_figstyle.py --
REM   python figures\make_figures.py                        (6)
REM   python ..\..\..\scripts\experiments\temporal_interior\fig_*.py
REM   nine included fig_*.py / toy*.py figure scripts                 (9)
REM Every figure is authored at \textwidth = 6.1677 in and included at
REM width=\textwidth, so the scale is 1.0 and rcParams sizes are the
REM printed point sizes.  Never include at a fractional width.
REM Five architecture/status diagrams are TikZ and compile in-document.
REM Set FTD_FIG_BACKEND=fallback if pdflatex-in-matplotlib is unavailable.
cd /d "%~dp0"
pdflatex -interaction=nonstopmode -halt-on-error -file-line-error semantic_ontology.tex
pdflatex -interaction=nonstopmode -halt-on-error -file-line-error semantic_ontology.tex
echo.
echo === build complete ===
