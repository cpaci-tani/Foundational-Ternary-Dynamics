@echo off
REM Build the FTD construction monograph (Perl-free; latexmk unavailable here).
REM pdflatex -> biber -> pdflatex x2.
setlocal
set DOC=monograph
set OPT=-interaction=nonstopmode -halt-on-error -file-line-error

echo [1/4] pdflatex (first pass)...
pdflatex %OPT% %DOC%.tex >nul || goto :err
echo [2/4] biber...
biber %DOC% >nul || goto :err
echo [3/4] pdflatex (second pass)...
pdflatex %OPT% %DOC%.tex >nul || goto :err
echo [4/4] pdflatex (final pass)...
pdflatex %OPT% %DOC%.tex >nul || goto :err

echo.
echo Done. See %DOC%.pdf
goto :eof
:err
echo BUILD FAILED - see %DOC%.log
exit /b 1
