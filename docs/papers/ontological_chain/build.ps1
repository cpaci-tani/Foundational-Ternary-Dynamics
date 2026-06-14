# Build the FTD Ontological Chain PDF.
# latexmk is unavailable on this MiKTeX (no Perl); explicit pass sequence instead.
$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot
pdflatex -interaction=nonstopmode ontological_chain.tex
biber ontological_chain
pdflatex -interaction=nonstopmode ontological_chain.tex
pdflatex -interaction=nonstopmode ontological_chain.tex
Write-Host "Built ontological_chain.pdf"
