# Build the FTD construction monograph.
# Perl-free pipeline (latexmk is unavailable on this MiKTeX install): run
# pdflatex, then biber, then pdflatex twice to settle citations, the ToC,
# cleveref, and bookmarks.
#
# Usage:  pwsh -File build.ps1     (from this directory)
$ErrorActionPreference = 'Stop'
$doc = 'monograph'
$opt = '-interaction=nonstopmode','-halt-on-error','-file-line-error'

Write-Host '[1/4] pdflatex (first pass)...'
pdflatex @opt "$doc.tex" | Out-Null
Write-Host '[2/4] biber...'
biber $doc | Out-Null
Write-Host '[3/4] pdflatex (second pass)...'
pdflatex @opt "$doc.tex" | Out-Null
Write-Host '[4/4] pdflatex (final pass)...'
pdflatex @opt "$doc.tex" | Out-Null

$line = (Select-String "$doc.log" -Pattern 'Output written on').Line
$ov   = @(Select-String "$doc.log" -Pattern 'Overfull \\hbox').Count
$un   = @(Select-String "$doc.log" -Pattern 'undefined').Count
Write-Host ''
Write-Host "Done. $line"
Write-Host "Overfull hboxes: $ov ; undefined references/citations: $un"
